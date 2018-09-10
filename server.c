#include "headers.h"

int getClientsAddr(slaveClients* sl, int* n){
    int udpFd=socket(PF_INET, SOCK_DGRAM, 0);
    msg a;
    int status=-1;
    struct sockaddr_in udpAddr={
            .sin_family=AF_INET,
            .sin_port=htons(UDPPORT),
            .sin_addr.s_addr= htonl(INADDR_BROADCAST)
    };
    socklen_t clientAddrLen;
    int rbuf = n[1];
    int ovl = -1;
    udpFd= socket(PF_INET, SOCK_DGRAM, 0);
    setsockopt(udpFd, SOL_SOCKET, SO_BROADCAST, &ovl, sizeof(ovl));
    sendto(udpFd, &rbuf, sizeof(rbuf), 0, (struct sockaddr *) &udpAddr, sizeof(udpAddr)); //заявляем о себе в бродкаст
    int summThreadsFromChld=0;
    for (int i=0; i < n[0]; i++) {
        sl[i].addr.sin_family=AF_INET,
        sl[i].addr.sin_port=htons(UDPPORT),
        sl[i].addr.sin_addr.s_addr= htonl(INADDR_ANY);
        sl[i].tcpAddr.sin_family=AF_INET;
        sl[i].tcpAddr.sin_port=0;
        sl[i].tcpAddr.sin_addr.s_addr= htonl(INADDR_ANY);
        clientAddrLen=sizeof(sl[i].addr);
        for(int u=0; u<5000; u++){
            if((status=recvfrom(udpFd, &a, sizeof(a), MSG_DONTWAIT, (struct sockaddr*) &sl[i].addr, &clientAddrLen))>=0) break; //ждем сообщения4
            usleep(100);
        }
        if(status<0) return-2;
        summThreadsFromChld+=a.threadNum;
        printf("SUMM TREADS %d\n",summThreadsFromChld);
        sl[i].threadNum=a.threadNum;
        sl[i].tcpAddr.sin_port=a.tcpAddr.sin_port;
        sl[i].tcpAddr.sin_addr=sl[i].addr.sin_addr;
        //printf("rcv %d %lu\n",i, sl[i].addr.sin_addr.);
        sl[i].tcpFd = socket(PF_INET, SOCK_STREAM, 0);
    }
    if(summThreadsFromChld!=n[1]){
        printf("error! thread num mismatch!");
        return 0;
    }
    printf("rcv-snd handshake\n");
    return 1;
}

int* input(int argc, char** argv){
    char *endptr, *str, *str2;
    long val, val2;
    int* rt=malloc(2*sizeof(int));
    if (argc < 2) {
        fprintf(stderr, "Slave not stated, interpreting as 1 \n");
        rt[0]=1;
        rt[1]=1;
        return rt;
    }
    str = argv[1];
    str2=argv[2];
    errno = 0;    /* To distinguish success/failure after call */
    val = strtol(str, &endptr, 10);
    val2=strtol(str2, &endptr, 10);
    /* Check for various possible errors */
    if ((errno == ERANGE && (val2 == INT_MAX || val2 == INT_MIN || val == INT_MAX || val == INT_MIN))
        || (errno != 0 && val == 0)|| (errno != 0 && val2 == 0)) {
        perror("range");
        return NULL;
    }
    if (endptr == str) {
        fprintf(stderr, "Slave num not stated, interpreting as 1 \n");
        return NULL;
    }
    /* If we got here, strtol() successfully parsed a number */
    rt[0]=val;
    rt[1]=val2;
    return rt;
}


int main(int argc, char** argv) {
    double a=0;
    double b=10000;
    int* n=input(argc, argv);
    if(!n[0] || !n[1]) return -1;
    borders bo[n[1]];
    printf("%d %d\n", n[0], n[1]);
    printf("Hello, World!\n");
    slaveClients* sl=calloc(n[0], sizeof(slaveClients));
    if(getClientsAddr(sl, n)!=1) return -1;
    int o;
    double result=0, back;
    struct timeval tv;
    int l=0;
    for(int i=0; i<n[0]; i++){
        int o=1;
        bo[i].a = a + (b - a) / n[1] * l;
        bo[i].b = a + (b - a) / n[1] * (l + sl[i].threadNum);
        l+=sl[i].threadNum;
        setsockopt(sl[i].tcpFd, SOL_SOCKET, SO_KEEPALIVE, &o, sizeof(o));
       /* tv.tv_sec = 60;
        tv.tv_usec = 0;
        setsockopt(sl[i].tcpFd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(tv));
        //write(sl[i].fd, &bo[i], sizeof(bo[i]));
        tv.tv_sec = 30;
        tv.tv_usec = 0;*/
      /*  setsockopt(sl[i].tcpFd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(tv));
        tv.tv_sec=0;
        tv.tv_usec=1000;*/
        sl[i].sk=-1;

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
    for(int i=0; i<n[0]; i++) {
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
