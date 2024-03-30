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
*/

#ifndef WEBORF_UTILS_H
#define WEBORF_UTILS_H

#include "types.h"
#include "options.h"

int list_dir(connection_t *connection_prop, char *html, unsigned int bufsize, bool parent);
void help();
void version();
void moo();
bool get_param_value(char *http_param, char *parameter, char *buf, ssize_t size,ssize_t param_len);
void daemonize();

#endif
