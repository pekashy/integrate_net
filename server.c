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

typedef struct message{
    char msg[8];
} message;

int main() {
    printf("Hello, World!\n");
    int udpFd=socket(PF_INET, SOCK_DGRAM, 0);
    struct sockaddr_in udpAddr={
            .sin_family=AF_INET,
            .sin_port=htons(4000),
            .sin_addr.s_addr=htonl(INADDR_BROADCAST)
    };
    int ovl=-1;
    setsockopt(udpFd, SOL_SOCKET, SO_BROADCAST, &ovl, sizeof(ovl));
    bind(udpFd, &udpAddr, sizeof(udpAddr));
    char buf = 0;
    //struct sockaddr* serverAddr;
    //socklen_t serverAddrLen=sizeof(*serverAddr);
    message msg={"gotcha"};
    printf("send\n");

    sendto(udpFd, &msg, sizeof(msg), 0, (struct sockaddr*) &udpAddr, sizeof(udpAddr));
    struct sockaddr_in recvAddr;
    unsigned int recvAddrLen=sizeof(recvAddr);
    printf("rcv\n");

    recvfrom(udpFd, &buf, MSG_WAITALL, sizeof(buf), &recvAddr, &recvAddrLen);
    printf("send-rcv handshake\n");


    return 0;
}