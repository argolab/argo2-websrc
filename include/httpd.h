#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <sys/wait.h>
#include <sys/fcntl.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/errno.h>
#include "prototypes.h"

#define SERVER_PROTOCOL "HTTP/1.0"
#define HTTPD_VERSION "Sanry-httpd/0.1"

char genbuf[256];
int restart_pending, shutdown_pending;
char fromhost[256];
char *stuff_ent, *query_string, *request_method;
char *http_cookie;
int content_length;

#define ACCESS_LOG	1
#define HTTPD_LOG	2
#define ERROR_LOG	3

#define ELOG_WARNING	1
#define ELOG_ERROR	2
#define ELOG_FATAL	3

