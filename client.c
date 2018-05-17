#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
//#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>

typedef struct slaveServers
{
    struct sockaddr_in addr;
    //struct socklen_t a;
} slaveServers;

int getClientsAddr(slaveServers* sl, int n){
    int udpFd=socket(PF_INET, SOCK_DGRAM, 0);
    struct sockaddr_in udpAddr={
            .sin_family=AF_INET,
            .sin_port=htons(4000),
            .sin_addr.s_addr= htonl(INADDR_ANY)
    };
    if(bind(udpFd, &udpAddr, sizeof(udpAddr))<0){
        printf("bind error");
        return 0;
    }
    socklen_t serverAddrLen;
    double buf = 8.0;
    int rbuf = 0;
    for (int i=0; i < n; i++) {
        sl[i].addr.sin_family=AF_INET,
        sl[i].addr.sin_port=htons(4000),
        sl[i].addr.sin_addr.s_addr= htonl(INADDR_ANY);
        serverAddrLen=sizeof(sl[i].addr);
        if(recvfrom(udpFd, &rbuf, sizeof(rbuf),MSG_WAITALL, (struct sockaddr*) &sl[i].addr, &serverAddrLen)<0) return -2; //ждем любого сообщения
        printf("rcv %lu\n", htonl(sl[i].addr.sin_addr.s_addr));
        if(sendto(udpFd, &buf, sizeof(buf), 0, (struct sockaddr*) &sl[i].addr, sizeof(sl[i].addr))<0){
            printf("msgsnd error\n");
            return -3;
        }
    }

    printf("rcv-snd handshake\n");
    return 0;
}

int main() {
    int n=2;
    printf("Hello, World!\n");
    slaveServers* sl=malloc(n*sizeof(slaveServers));
    getClientsAddr(sl, n);
    return 0;
}