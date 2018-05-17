#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>
#include <math.h>
#include <sys/sysinfo.h>
#include <errno.h>

#define EPS 1.e-5
#define FUNC 1/(sin(x*x)+2)

typedef struct borders{
    double a;
    double b;
} borders;

typedef struct core{
    int id;
    int n;
    cpu_set_t mask;
    int load;
} core;

typedef struct slaveServers
{
    struct sockaddr_in addr;
    int fd;
    //struct socklen_t a;
} slaveServers;
