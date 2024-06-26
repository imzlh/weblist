/*
Weborf
Copyright (C) 2007  Salvo "LtWorf" Tomaselli

Weborf is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

@author Salvo "LtWorf" Tomaselli <tiposchi@tiscali.it>
@author Giuseppe Pappalardo <pappalardo@dmi.unict.it>
@author Salvo Rinaldi <salvin@anche.no>
*/
#include "options.h"

#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <syslog.h>
#include <getopt.h>
#include <pthread.h>
#include <poll.h>
#include <time.h>


#include "listener.h"
#include "instance.h"
#include "queue.h"
#include "utils.h"
#include "mystring.h"
#include "types.h"
#include "cachedir.h"
#include "configuration.h"
#include "mynet.h"

#define _GNU_SOURCE

syn_queue_t queue;              //Queue for opened sockets

t_thread_info thread_info;

extern weborf_configuration_t weborf_conf;

pthread_attr_t t_attr;          //thread's attributes

pthread_key_t thread_key;            //key for pthread_setspecific

/**
Sets t_attr to make detached threads
and initializes pthread_keys
*/
static void init_thread_attr() {
    pthread_attr_init(&t_attr);
    pthread_attr_setdetachstate(&t_attr, PTHREAD_CREATE_DETACHED);
    pthread_key_create(&thread_key,NULL);

}

/**
Starts threads
Specify how many threads start.
*/
void init_threads(unsigned int count) {
    static long int id = 1;
    //t_free=MAXTHREAD;
    int effective=0,i;

    pthread_t t_id;//Unused var, thread's system id

    pthread_mutex_lock(&thread_info.mutex);
    //Check condition within the lock
    if (thread_info.count + count < MAXTHREAD) {

        //Start
        for (i = 1; i <= count; i++)
            if (pthread_create(&t_id, &t_attr, instance, (void *) (id++))==0) effective++;

        thread_info.count+=effective; // increases the count of started threads
#ifdef THREADDBG
        syslog(LOG_DEBUG, "There are %d free threads", thread_info.free);
#endif

#ifdef SERVERDBG
        if (effective!=count)
            syslog(LOG_CRIT,"Unable to launch the required threads");
#endif


    }
    pthread_mutex_unlock(&thread_info.mutex);
}

/**
This function inits the logger.
Will use syslogd
*/
static void init_logger() {
    openlog("WebList", LOG_ODELAY, LOG_DAEMON);
#ifdef SERVERDBG
    syslog(LOG_INFO, "Starting server...");
#endif

}


static void init_thread_info() {
    //Init thread_info
    pthread_mutex_init(&thread_info.mutex, NULL);
    thread_info.count=0;
    thread_info.free=0;
}


/**
This function, terminates threads if there are too many free threads.

It only takes action every THREADCONTROL seconds.
*/
static void t_shape() {
    static time_t last_action = 0;

    if (last_action + 10 > time(NULL)) {
        return;
    }
    last_action = time(NULL);

    if (thread_info.free > MAXFREETHREAD) { //Too many free threads, terminates one of them
        //Write the termination order to the queue, the thread who will read it, will terminate
        q_put(&queue,-1);
    }
}


/**
 * Set quit action on SIGTERM and SIGINT
 * and prints the internal status on SIGUSR1
 * */
static void init_signals() {
    //Handle SIGINT and SIGTERM
    signal(SIGINT, quit);
    signal(SIGTERM, quit);

    //Prints queue status with this signal
    signal(SIGUSR1, print_queue_status);
}

int main(int argc, char *argv[]) {
    int s, s1;          //Socket descriptors

    init_logger();
    init_thread_info();

    configuration_load(argc,argv);

    if (weborf_conf.is_inetd) inetd();

    s = net_create_server_socket();
    net_bind_and_listen(s);

    //init the queue for opened sockets
    if (q_init(&queue, MAXTHREAD + 1) != 0)
        exit(NOMEM);

    //Starts the 1st group of threads
    init_thread_attr();
    init_threads(INITIALTHREAD);
    init_signals();

    struct pollfd poll_fds[1];
    poll_fds[0].fd = s;
    poll_fds[0].events = POLLIN;

    while (1) {
        if (poll(poll_fds, 1, 1000 * THREADCONTROL) == -1) {
#ifdef SERVERDBG
            syslog(LOG_ERR, "Error polling server socket: %d", errno);
#endif
            exit(11);
        }


        t_shape();

        s1 = accept(s, NULL,NULL);

        if (s1 >= 0 && q_put(&queue, s1)!=0) { //Adds s1 to the queue
#ifdef REQUESTDBG
            syslog(LOG_ERR,"Not enough resources, dropping connection...");
#endif
            close(s1);
        }

        //Start new thread if needed
        if (thread_info.free <= LOWTHREAD && thread_info.free<MAXTHREAD) { //Need to start new thread
            if (thread_info.count + INITIALTHREAD < MAXTHREAD) { //Starts a group of threads
                init_threads(INITIALTHREAD);
            } else { //Can't start a group because the limit is close, starting less than a whole group
                init_threads(MAXTHREAD - thread_info.count);
            }
        }

    }
    return 0;

}

/**
SIGINT and SIGTERM signal handler
*/
void quit() {
#ifdef SERVERDBG
    syslog(LOG_INFO, "Stopping server...");
#endif

    closelog();
    exit(0);
}

/**
Will print the internal status of the queue.
This function is triggered by SIGUSR1 signal.
*/
void print_queue_status() {

    //Lock because the values are read many times and it's needed that they have the same value all the times

    if ( pthread_mutex_trylock(&queue.mutex)==0) {
        printf("Queue is unlocked\n");
        pthread_mutex_unlock(&queue.mutex);
    } else {
        printf("Queue is locked\n");
    }


    if ( pthread_mutex_trylock(&thread_info.mutex)==0) {
        printf("thread_info is unlocked\n");
        pthread_mutex_unlock(&thread_info.mutex);
    } else {
        printf("thread_info is locked\n");
    }

    pthread_mutex_lock(&thread_info.mutex);
    printf("=== Queue ===\ncount:      %d\t"
           "size:       %d\n"
           "head:       %d\t"
           "tail:       %d\n"
           "wait_data:  %d\t"
           "wait_space: %d\n"
           "=== Threads ===\n"
           "Maximum:    %d\n"
           "Started:    %d\n"
           "Free:       %d\n"
           "Busy:       %d\n",
           queue.num,queue.size,
           queue.head,queue.tail,
           queue.n_wait_dt,queue.n_wait_sp,
           MAXTHREAD,thread_info.count,
           thread_info.free,thread_info.count-thread_info.free
          );
    pthread_mutex_unlock(&thread_info.mutex);
}
