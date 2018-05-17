#include "headers.h"

int getClientsAddr(slaveServers* sl, int n){
    int udpFd=socket(PF_INET, SOCK_DGRAM, 0);
    msg a;
    struct sockaddr_in udpAddr={
            .sin_family=AF_INET,
            .sin_port=htons(4000),
            .sin_addr.s_addr= htonl(INADDR_ANY)
    };
    struct sockaddr_in tcpAddr = {
            .sin_family=AF_INET,
            .sin_port=0,
            .sin_addr.s_addr=htonl(INADDR_ANY)
    };

    if(bind(udpFd, &udpAddr, sizeof(udpAddr))<0){
        printf("bind error");
        return 0;
    }
    socklen_t serverAddrLen, tcpAddrLen;
    double buf = 8.0;
    int rbuf = 0;
    for (int i=0; i < n; i++) {
        sl[i].addr.sin_family=AF_INET,
        sl[i].addr.sin_port=htons(4000),
        sl[i].addr.sin_addr.s_addr= htonl(INADDR_ANY);
        sl[i].tcpAddr.sin_family=AF_INET;
        sl[i].tcpAddr.sin_port=0;
        sl[i].tcpAddr.sin_addr.s_addr= htonl(INADDR_ANY);

        /*msg.tcpAddr.sin_family=AF_INET,
        msg.tcpAddr.sin_port=htons(4500),
        msg.tcpAddr.sin_addr.s_addr= htonl(INADDR_ANY);*/

        serverAddrLen=sizeof(sl[i].addr);
        if(recvfrom(udpFd, &rbuf, sizeof(rbuf), MSG_WAITALL, (struct sockaddr*) &sl[i].addr, &serverAddrLen)<0) return -2; //ждем любого сообщения
        printf("rcv %d %lu\n",i, sl[i].addr.sin_addr.s_addr);
        sl[i].tcpFd = socket(PF_INET, SOCK_STREAM, 0);
        bind(sl[i].tcpFd, (struct sockaddr*) &sl[i].tcpAddr, sizeof(sl[i].tcpAddr));
        tcpAddrLen=sizeof(sl[i].tcpAddr);
        getsockname(sl[i].tcpFd, &sl[i].tcpAddr, &tcpAddrLen);
        a.tcpAddr=sl[i].tcpAddr;
        listen(sl[i].tcpFd, 256);

        if(sendto(udpFd, &a, sizeof(msg), 0, (struct sockaddr*) &sl[i].addr, sizeof(sl[i].addr))<0){
            printf("msgsnd error\n");
            return -3;
        }
    }
    printf("rcv-snd handshake\n");
    return 1;
}

int main() {
    double a=0;
    double b=500;
    int n=1;
    borders bo[n];
    printf("Hello, World!\n");
    slaveServers* sl=calloc(n, sizeof(slaveServers));
    if(getClientsAddr(sl, n)!=1) return -1;
    int o;
    int status=-1;
    double result=0, back;
    struct timeval tv;
    for(int i=0; i<n; i++){
        //sl[i].fd=socket(PF_INET, SOCK_STREAM, 0);
        //bind(sl[i].fd, (struct sockaddr*) &sl[i].addr, sizeof(sl[i].addr));
        bo[i].a = a + (b - a) / n * i;
        bo[i].b = a + (b - a) / n * (i + 1);
        setsockopt(sl[i].tcpFd, SOL_SOCKET, SO_KEEPALIVE, &o, sizeof(o));
        tv.tv_sec = 2;
        tv.tv_usec = 0;
        setsockopt(sl[i].tcpFd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(tv));
        //write(sl[i].fd, &bo[i], sizeof(bo[i]));
        sl[i].sk=-1;
        if((sl[i].sk=accept(sl[i].tcpFd, NULL, NULL))<0) return 0;

        o=write(sl[i].sk, &bo[i], sizeof(bo[i]));
        printf("%f %f %d\n", bo[i].a, bo[i].b, o);

    }
    for(int i=0; i<n; i++) {
        status = read(sl[i].sk, &back, sizeof(back));
        if (status<0) {
            printf ("error: client not found\n");
            return -1;
        }
        result+=back;
        close(sl[i].tcpFd);
    }
    printf("%e\n", result);
    return 0;
}