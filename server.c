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
    struct sockaddr_in addr;
} message;

int main() {
    printf("Hello, World!\n");
    int udpFd = socket(PF_INET, SOCK_DGRAM, 0);
    int tcpFd= socket(PF_INET, SOCK_STREAM, 0);
    message msg;
    struct sockaddr_in udpAddr = {
            .sin_family=AF_INET,
            .sin_port=htons(4000),
            .sin_addr.s_addr=htonl(INADDR_BROADCAST)
    };
    struct sockaddr_in tcpAddr;

    int ovl = -1;
    setsockopt(udpFd, SOL_SOCKET, SO_BROADCAST, &ovl, sizeof(ovl));
    int rbuf = getpid();
    printf("pid %d", rbuf);
    int buf = 0;

    int status = -1;

    sendto(udpFd, &rbuf, sizeof(rbuf), 0, (struct sockaddr *) &udpAddr,
           sizeof(udpAddr)); //заявляем о себе в бродкаст
    printf("send\n");

    struct sockaddr_in recvAddr;//сюда пишем адрес
    unsigned int recvAddrLen = sizeof(recvAddr);

    if(recvfrom(udpFd, &msg, sizeof(msg), MSG_DONTWAIT, &recvAddr,
                &recvAddrLen)<0){
        printf("error connecting to client\n");
        return 0;
    }
    printf("send-rcv handshake\n");
    tcpAddr=msg.addr;
    bind(tcpFd, (struct sockaddr*) &tcpAddr, sizeof(tcpAddr));
    listen(tcpFd, 256);

    int sk=accept(tcpFd,(struct sockaddr*) &tcpAddr, sizeof(tcpAddr));

    printf("tcp handshake %d\n", sk);

    return 0;
}
