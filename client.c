#include "headers.h"

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
        sl[i].fd=socket(PF_INET, SOCK_STREAM, 0);
    }
    printf("rcv-snd handshake\n");
    return 1;
}

int main() {
    int a=0;
    int b=500;
    int n=2;
    borders bo[n];
    printf("Hello, World!\n");
    slaveServers* sl=calloc(n, sizeof(slaveServers));
    if(getClientsAddr(sl, n)!=1) return -1;
    int o;
    int status=-1;
    double result=0, back;
    struct timeval tv;
    for(int i=0; i<n; i++){
        bo[i].a = a + (b - a) / n * i;
        bo[i].b = a + (b - a) / n * (i + 1);
        setsockopt(sl[i].fd, SOL_SOCKET, SO_KEEPALIVE, &o, sizeof(o));
        tv.tv_sec = 0;
        tv.tv_usec = 0;
        setsockopt(sl[i].fd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(tv));
        connect(sl[i].fd, (struct sockaddr*) &sl[i].addr, sizeof(sl[i].addr));
        write(sl[i].fd, &bo[i], sizeof(bo[i]));
    }
    for(int i=0; i<n; i++) {
        status = read(sl[i].fd, &back, sizeof(back));
        if (status<0) {
            printf ("error: client not found\n");
            return -1;
        }
        result+=back;
        close(sl[i].fd);
    }
    printf("%e\n", result);
    return 0;
}