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
    int n=1;
    printf("Hello, World!\n");
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
    struct sockaddr_in serverAddr={
            .sin_family=AF_INET,
            .sin_port=htons(4000),
            .sin_addr.s_addr= htonl(INADDR_ANY)
    };
    socklen_t serverAddrLen=sizeof(serverAddr);

    int buf = 8;
    int rbuf = 0;

    int status =-1;

   /* recvfrom(udpFd, &buf, sizeof(buf),MSG_WAITALL, serverAddr, &serverAddrLen); //ждем любого сообщения
    printf("rcv\n");
    if(sendto(udpFd, &buf, sizeof(buf), 0, (struct sockaddr*) serverAddr, sizeof(*serverAddr))<0){
        printf("msgsnd error, possibly sigpipe\n");
        return 0;
    } //*/
    for (int i=n; i > 0; ) {
        if(recvfrom(udpFd, &rbuf, sizeof(rbuf),MSG_WAITALL, (struct sockaddr*) &serverAddr, &serverAddrLen)<0) return -2; //ждем любого сообщения
        printf("rcv\n");
        if(sendto(udpFd, &buf, sizeof(buf), 0, (struct sockaddr*) &serverAddr, sizeof(serverAddr))<0){
            printf("msgsnd error\n");
            return -3;
        } //

        //(sendto(fdUdp, &msgC, sizeof(msgC), 0, (struct sockaddr*)&addr, sizeof(addr)));

       // Msg msg = {};
        //errno = 0;
        /*clients[nClients].addrLen = sizeof(clients[nClients].addr);
        ret = recvfrom(fdUdp, &msg,
                       sizeof(msg), MSG_DONTWAIT, (struct sockaddr*)&clients[nClients].addr, &clients[nClients].addrLen);

        if ( ret == -1 && errno != EAGAIN && errno != EWOULDBLOCK)
            return -1;
        else if (ret != -1)
        {
            clients[nClients].nThreads = msg.nThreads;
            clients[nClients].addr.sin_port =  msg.port;
            residue -= clients[nClients].nThreads;
            nClients++;
        }*/
        i--;
    }

    printf("rcv-snd handshake\n");
    return 0;
}