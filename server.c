#include "headers.h"

int getClientsAddr(slaveClients* sl, int n){
    int udpFd=socket(PF_INET, SOCK_DGRAM, 0);
    msg a;
    int status=-1;
    struct sockaddr_in udpAddr={
            .sin_family=AF_INET,
            .sin_port=htons(UDPPORT),
            .sin_addr.s_addr= htonl(INADDR_BROADCAST)
    };
    struct sockaddr_in tcpAddr = {
            .sin_family=AF_INET,
            .sin_port=UDPPORT+1,
            .sin_addr.s_addr=htonl(INADDR_ANY)
    };

    /*if(bind(udpFd, &udpAddr, sizeof(udpAddr))<0){
        printf("bind error");
        return 0;
    }*/
    struct timeval tv;
    socklen_t clientAddrLen, tcpAddrLen;
    double buf = 8.0;
    int rbuf = 0;
    int ovl = -1;
    udpFd= socket(PF_INET, SOCK_DGRAM, 0);
    setsockopt(udpFd, SOL_SOCKET, SO_BROADCAST, &ovl, sizeof(ovl));
    sendto(udpFd, &rbuf, sizeof(rbuf), 0, (struct sockaddr *) &udpAddr, sizeof(udpAddr)); //заявляем о себе в бродкаст
    for (int i=0; i < n; i++) {
        sl[i].addr.sin_family=AF_INET,
        sl[i].addr.sin_port=htons(UDPPORT),
        sl[i].addr.sin_addr.s_addr= htonl(INADDR_ANY);
        sl[i].tcpAddr.sin_family=AF_INET;
        sl[i].tcpAddr.sin_port=0;
        sl[i].tcpAddr.sin_addr.s_addr= htonl(INADDR_ANY);
        clientAddrLen=sizeof(sl[i].addr);
        for(int u=0; u<500; u++){
            if((status=recvfrom(udpFd, &a, sizeof(a), MSG_DONTWAIT, (struct sockaddr*) &sl[i].addr, &clientAddrLen))>=0) break; //ждем сообщения4
            usleep(1000);
        }
        if(status<0) return-2;
        sl[i].tcpAddr.sin_port=a.tcpAddr.sin_port;
        sl[i].tcpAddr.sin_addr=sl[i].addr.sin_addr;
        //sl[i].tcpAddr.sin_port=a.tcpAddr.sin_port;
        printf("rcv %d %lu\n",i, sl[i].addr.sin_addr);
        sl[i].tcpFd = socket(PF_INET, SOCK_STREAM, 0);
        //sl[i].tcpAddr.sin_port=29228;
        /*if(bind(sl[i].tcpFd, (struct sockaddr*) &sl[i].tcpAddr, sizeof(sl[i].tcpAddr))){
            printf("bind error try another port");
           // return -3;
        }/*
        tcpAddrLen=sizeof(sl[i].addr);*/
        //getsockname(sl[i].tcpFd, &sl[i].addr, &tcpAddrLen);
        //a.tcpAddr=sl[i].tcpAddr;
        /*if(sendto(udpFd, &a, sizeof(msg), 0, (struct sockaddr*) &sl[i].addr, sizeof(sl[i].addr))<0){
            printf("msgsnd error\n");
            return -3;
        }*/
    }
    printf("rcv-snd handshake\n");
    return 1;
}

int input(int argc, char** argv){
    char *endptr, *str;
    long val;
    if (argc < 2) {
        fprintf(stderr, "Slave not stated, interpreting as 1 \n");
        return 1;
    }
    str = argv[1];
    errno = 0;    /* To distinguish success/failure after call */
    val = strtol(str, &endptr, 10);
    /* Check for various possible errors */
    if ((errno == ERANGE && (val == INT_MAX || val == INT_MIN))
        || (errno != 0 && val == 0)) {
        perror("range");
        return 0;
    }
    if (endptr == str) {
        fprintf(stderr, "Slave num not stated, interpreting as 1 \n");
        return 1;
    }
    /* If we got here, strtol() successfully parsed a number */
    return (int) val;
}


int main(int argc, char* argv) {
    double a=0;
    double b=500;
    int n=input(argc, argv);
    borders bo[n];
    printf("Hello, World!\n");
    slaveClients* sl=calloc(n, sizeof(slaveClients));
    if(getClientsAddr(sl, n)!=1) return -1;
    int o;
    double result=0, back;
    struct timeval tv;
    for(int i=0; i<n; i++){
        int o=1;

        bo[i].a = a + (b - a) / n * i;
        bo[i].b = a + (b - a) / n * (i + 1);
        setsockopt(sl[i].tcpFd, SOL_SOCKET, SO_KEEPALIVE, &o, sizeof(o));
        tv.tv_sec = 60;
        tv.tv_usec = 0;
        setsockopt(sl[i].tcpFd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(tv));
        //write(sl[i].fd, &bo[i], sizeof(bo[i]));
        sl[i].sk=-1;
        tv.tv_sec = 30;
        tv.tv_usec = 0;
        setsockopt(sl[i].tcpFd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(tv));
        tv.tv_sec=0;
        tv.tv_usec=1000;
        for (int k=0; (o=connect(sl[i].tcpFd, (struct sockaddr*) &sl[i].tcpAddr, sizeof(sl[i].tcpAddr))==-1 && k<1000); k++){
            //  tv.tv_sec = 1;
            //  tv.tv_usec = 0;
        }
        o=write(sl[i].tcpFd, &bo[i], sizeof(bo[i]));
        if (o<0) {
            printf ("error: server is dead\n");
            return -1;
        }
    }
    for(int i=0; i<n; i++) {
        //printf("%f %f %d\n", bo[i].a, bo[i].b, o);
        o = read(sl[i].tcpFd, &back, sizeof(back));
        if (o<=0) {
            printf ("result read error\n");
            return -1;
        }
        result+=back;
        close(sl[i].tcpFd);
    }
    printf("%e\n", result);
    return 0;
}
