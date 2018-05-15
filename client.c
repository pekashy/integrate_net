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


typedef struct slaveServers
{
    struct sockaddr_in udpAddr;
    struct sockaddr_in tcpAddr;
    int thrNum;
    socklen_t addrLen;
    int sk;
    int tcpFd;
    int udpFd;
} slaveServers;

typedef struct message{
    struct sockaddr_in addr;
} message;

int main() {
    int n=2;
    slaveServers sl[n];
    message msg[n];

    printf("Hello, World!\n");

    for(int a=0; a<n; a++) {
        sl[a].udpFd=socket(PF_INET, SOCK_DGRAM, 0);
        sl[a].tcpFd= socket(PF_INET, SOCK_STREAM, 0);
        if(bind(sl[a].udpFd, &sl[a].udpAddr, sizeof(sl[a].udpAddr))<0){
            printf("bind error");
            return 0;
        }
        if(bind(sl[a].tcpFd, &sl[a].tcpAddr, sizeof(sl[a].tcpAddr))<0){
            printf("bind error");
            return 0;
        }
        sl[a].udpAddr = {
                .sin_family=AF_INET,
                .sin_port=htons(4000),
                .sin_addr.s_addr= htonl(INADDR_ANY)
        };
        sl[a].tcpAddr = {
                .sin_family = AF_INET,
                .sin_port = 0,
                .sin_addr.s_addr = htonl(INADDR_ANY)
        };
    }

    int rbuf = 0;
    for (int i=0; i < n; i++) {
        sl[i].tcpFd = socket(PF_INET, SOCK_STREAM, 0);
        if(recvfrom(sl[i].udpFd, &rbuf, sizeof(rbuf),MSG_WAITALL, (struct sockaddr*) &sl[i].udpAddr, &sl[i].addrLen)<0) return -2; //ждем любого сообщения
        printf("rcv\n");
        bind(sl[i].tcpFd, (struct sockaddr*)&sl[i].tcpAddr, sizeof(sl[i].tcpAddr));
        msg[i].addr=sl[i].tcpAddr;
        if(sendto(sl[i].udpFd, &msg, sizeof(msg), MSG_WAITALL, (struct sockaddr*) &sl[i].udpAddr, sizeof(sl[i].addrLen))<0){
            printf("msgsnd error\n");
            return -3;
        }

        //

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
        printf("rcv-snd handshake\n");

    }

    printf("rcv-snd handshake\n");
    return 0;
}