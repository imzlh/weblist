/*
Weborf
Copyright (C) 2007-2020  Salvo "LtWorf" Tomaselli

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

 */
#include "options.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <stdbool.h>
#include <netdb.h>
#include <dirent.h>
#include <unistd.h>
#include <syslog.h>
#include <signal.h>
#include <string.h>

#include "mystring.h"
#include "utils.h"
#include "embedded_auth.h"

/**
 * This function creates an URI encoded version of the string origin
 **/
static void uri_encode(char *dest, size_t size, const char *origin) {
    if (size == 0 || dest == NULL || origin == NULL)
        return;

    dest[0] = '\0';

    size_t len = strlen(origin);
    size_t rlen = 0;

    char *format;
    for (size_t i = 0; i < len; i++) {
        //Encode forbidden characters
        switch (origin[i]) {
            case '!':
            case '*':
            case '\'':
            case '(':
            case ')':
            case ';':
            case ':':
            case '@':
            case '&':
            case '=':
            case '+':
            case '$':
            case ',':
            case '/':
            case '?':
            case '#':
            case '[':
            case ']':
            case '%':
            case '>':
            case '<':
            case '"':
            case ' ':
            case '\\':
                format = "%%%02x";
                break;
            default:
                format = "%c";
                break;
        }
        if (size - rlen <= 2)
            return;
        rlen += sprintf(dest + rlen, format, origin[i]);
    }
}

/**
 * This function encodes the characters in 'origin' to &code; form into 'encoded'
 **/
static void html_encode(char *dest, size_t size, const char *origin) {
    if (size == 0 || dest == NULL || origin == NULL) return;

    dest[0] = '\0';

    size_t len = 0;
    char *code;
    size_t codesize;
    char ni[2] = "X";
    size_t origin_len = strlen(origin);

    for (size_t i = 0; i < origin_len; i++) {
        ni[0] = origin[i];
        switch (origin[i]) {
        case '&':
            code = "&amp;";
            codesize = strlen("&amp;");
            break;
        case '<':
            code = "&lt;";
            codesize = strlen("&lt;");
            break;
        case '>':
            code = "&gt;";
            codesize = strlen("&gt;");
            break;
        case ' ':
            code = "&nbsp;";
            codesize = strlen("&nbsp;");
            break;
        case '"':
            code = "&quot;";
            codesize = strlen("&quot;");
            break;
        default:
            code = ni;
            codesize = 1;
            break;
        }
        if (len + codesize >= size)
            return;
        strcpy(dest + len, code);
        len += codesize;
    }
}

/**
This function reads the directory dir, putting inside the html string an html
page with links to all the files within the directory.

Buffer for html must be allocated by the calling function.
bufsize is the size of the buffer allocated for html
parent is true when the dir has a parent dir

Returns the size of the HTML.

-1: unable to open the file
-2: out of memory

*/
int list_dir(connection_t *connection_prop, char *html, unsigned int bufsize, bool parent) {
    int pagesize=0; //Written bytes on the page
    int maxsize = bufsize - 1; //String's max size
    int printf_s;
    char *measure; //contains measure unit for file's size (B, KiB, MiB)
    int counter = 0;
    int errcode = 0;

    char path[INBUFFER]; //Buffer to contain element's absolute path

    struct dirent **namelist = NULL;
    counter = scandir(connection_prop->strfile, &namelist, 0, alphasort);
    char *name_html = malloc(ESCAPED_FNAME_LEN);
    char *escaped_dname = malloc(ESCAPED_FNAME_LEN);

    if (counter <0) { //Open not succesfull
        errcode = -1;
        goto escape;
    }

    //Specific header table)
    pagesize=printf_s=snprintf(html+pagesize,maxsize, "%s", HTMLHEAD "<h><c>名称</c><c>大小</c><c>最后更新</c></h>");
    maxsize-=printf_s;

    //Cycles trough dir's elements
    int i;
    struct tm ts;
    struct stat f_prop; //File's property
    char last_modified[URI_LEN];

    //Print link to parent directory, if there is any
    if (parent) {
        printf_s=snprintf(html+pagesize,maxsize,"<d><c><a href=\"../\">上一级目录</a></c><c>-</c><c>-</c></d>");
        maxsize-=printf_s;
        pagesize+=printf_s;
    }

    for (i=0; i<counter; i++) {
        //Skipping hidden files
        if (namelist[i]->d_name[0] == '.' || errcode) {
            free(namelist[i]);
            continue;
        }

        snprintf(path, INBUFFER,"%s/%s", connection_prop->strfile, namelist[i]->d_name);

        //Stat on the entry
        stat(path, &f_prop);
        int f_mode = f_prop.st_mode; //Get's file's mode

        //get last modified
        localtime_r(&f_prop.st_mtime,&ts);
        strftime(last_modified,URI_LEN, "%y-%m-%d %H:%M", &ts);

        html_encode(name_html, ESCAPED_FNAME_LEN, namelist[i]->d_name);
        uri_encode(escaped_dname, ESCAPED_FNAME_LEN, namelist[i]->d_name);

        if (S_ISREG(f_mode)) { //Regular file

            //Table row for the file

            //Scaling the file's size
            unsigned long long int size = f_prop.st_size;
            if (size < 1024) {
                measure="B";
            } else if ((size = (size / 1024)) < 1024) {
                measure="KB";
            } else if ((size = (size / 1024)) < 1024) {
                measure="MB";
            } else {
                size = size / 1024;
                measure="GB";
            }
            
            printf_s=snprintf(html+pagesize,maxsize,
                              "<f><c><a href=\"%s\">%s</a></c><c>%llu%s</c><c>%s</c></f>\n",
                              escaped_dname, name_html, size, measure,last_modified);
            maxsize-=printf_s;
            pagesize+=printf_s;

        } else if (S_ISDIR(f_mode)) { //Directory entry
            //Table row for the dir
            printf_s=snprintf(html+pagesize,maxsize,
                              "<d><c><a href=\"%s/\">%s/</a></c><c>-</c><c>%s</c></d>\n",
                              escaped_dname, name_html,last_modified);
            maxsize-=printf_s;
            pagesize+=printf_s;
        }

        free(namelist[i]);
        if (maxsize <= 0) {
            errcode = -2; // Out of memory
        }
    }

escape:
    free(name_html);
    free(escaped_dname);
    free(namelist);
    if (errcode == 0) {
        printf_s=snprintf(html+pagesize,maxsize, "%s", HTMLFOOT);
        pagesize+=printf_s;
        return pagesize;
    } else
        return errcode;
}

/**
Prints version information
*/
void version() {
    printf("WebList %s Main", VERSION);
    exit(0);
}

/**
Prints command line help
 */
void help() {

        printf("Usage: weblist [OPTIONS]\n"
           "\tStart the weblist webserver\n\n"
#ifdef IPV6
           "\t(*) Compiled for IPv6\n"
#else
           "\t(*) Compiled for IPv4\n"
#endif

#ifdef WEBDAV
           "\t(*) Has webdav support\n"
#endif

           " # Default port is        %s\n"
           " # Default base directory %s\n"
           " # Signature used         %s\n\n", PORT,BASEDIR,SIGNATURE);

    printf("  -a, --auth    followed by absolute path of the program to handle authentication\n"
           "  -b, --basedir followed by absolute path of basedir\n"
           "  -C, --cache   sets the directory to use for cache files\n"
           "  -c, --cgi     list of cgi files and binary to execute them comma-separated\n"
           "  -h, --help    display this help and exit\n"
           "  -I, --index   list of index files, comma-separated\n"
           "  -i, --ip  followed by IP address to listen (dotted format)\n"
           "  -k, --caps    lists the capabilities of the binary\n"
           "  -p, --port    followed by port number to listen\n"
           "  -T  --inetd   must be specified when using weblist with inetd or xinetd\n"
           "  -t  --tar     will send the directories as .tar.gz files\n"
           "  -V, --virtual list of virtualhosts in the form host=basedir, comma-separated\n"
           "  -v, --version print program version\n"
#ifdef HAVE_LIBSSL
           "  -S, --cert    the certificate to use\n"
           "  -K, --key     the private key to use with the certificate\n"
#endif
           "  -Y, --yesexec enables CGI\n"
           "\n");
    exit(0);
}


/**
This function retrieves the value of an http field within the header
http_param is the string containing the header
parameter is the searched parameter
buf is the buffer where copy the value
size, maximum size of the buffer
param_len =length of the parameter

Returns false if the parameter isn't found, or true otherwise
*/
bool get_param_value(char *http_param, char *parameter, char *buf, ssize_t size,ssize_t param_len) {
    char *val = strstr(http_param, parameter); //Locates the requested parameter information

    if (val == NULL) { //No such field
        return false;
    }

    /*
     * It is very important for this line to be here, for security reasons.
     * It moves the pointer forward, assuming "Field: Value\r\n"
     * If the field is malformed like "Field0\r\n" the subsequent strstr
     * will fail and the function will return false.
     * Moving this line after the next strstr would introduce a security
     * vulnerability.
     * The strstr will not cause a segfault because at this point the header
     * string must at least terminate with "\r\n\r", the last '\r' is changed to 0
     * so there is enough space to perform the operation
     * */
    val += param_len + 2; //Moves the begin of the string to exclude the name of the field

    char *field_end = strstr(val, "\r\n"); //Searches the end of the parameter
    if (field_end==NULL) {
        return false;
    }

    if ((field_end - val + 1) < size) { //If the parameter's length is less than buffer's size
        memcpy(buf, val, field_end - val);
    } else { //Parameter string is too long for the buffer
        return false;
    }
    buf[field_end - val] = 0; //Ends the string within the destination buffer

    return true;
}


