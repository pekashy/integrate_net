#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>


int main() {
    printf("Hello, World!\n");
    int udpFd=socket(PF_INET, SOCK_DGRAM, 0);
    struct sockaddr_in udpAddr={
            .sin_family=AF_INET,
            .sin_port=htons(4000),
            .sin_addr.s_addr= htonl(INADDR_ANY)
    };
    bind(udpFd, &udpAddr, sizeof(udpAddr));
    char buf = 0;
    struct sockaddr serverAddr={};
    socklen_t serverAddrLen=sizeof(serverAddr);
    printf("rcv\n");
    recvfrom(udpFd, &buf, sizeof(buf),0, &serverAddr, &serverAddrLen);
    printf("send-rcv handshake\n");
    return 0;
}