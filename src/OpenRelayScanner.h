/**
 * Created by GKHighElf - gkhighelf@gmail.com
 * in near 2000 year, for self education and use.
**/

#include <signal.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
//#include <arpa/inet.h>

#include <errno.h>
#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif

#include <sys/times.h>

#include "fdwatch.h"

#ifndef SHUT_WR
#define SHUT_WR 1
#endif

#ifndef HAVE_INT64T
typedef long long int64_t;
#endif

