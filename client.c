#include "headers.h"
#include <sys/poll.h>


void* threadFunc(void* b){

    borders* bord = (borders*) b;
    double * summ;
    double ftrp=0;
    summ= malloc((sizeof(double)));
    double a= bord->a;
    long long n=(long long) ((bord->b-a)/EPS);
    long long count=0;
    for(double x=a+EPS; count<n; count=count+1){
        ftrp+=FUNC*(EPS);
        x+=EPS;
    }
    printf("proc %lu; proc %d; a %f\n", pthread_self(), sched_getcpu(), a);
    *summ=ftrp;
    return summ;
}

double integrate(borders boo, int n) {
    cpu_set_t mask;
    int mCpu, mCore;
    int procNum = get_nprocs();

    int coreIdMax = -1;
    char c[33];

    const char cheatcode[] = "fgrep -e 'processor' -e 'core id' /proc/cpuinfo";
    FILE *cpuinfo_file = popen(cheatcode, "r");
    FILE *crs = popen("grep 'core id' /proc/cpuinfo | grep -Eo '[0-9]{1,4}' | sort -rn | head -n 1",
                      "r"); //getting maximum cpuId    //sched_setscheduler(pthread_self(), SCHED_FIFO, NULL);

    if (!cpuinfo_file || !crs) {
        perror("error opening cpuinfo file");
        exit(-1);
    }
    fscanf(crs, "%d", &coreIdMax);
    if (coreIdMax < 0) {
        printf("core num parsing error");
        exit(-1);
    }

    core *cpu = malloc(sizeof(core) * (coreIdMax + 1));
    for (int y = 0; y < coreIdMax + 1; y++) {
        cpu[y].id = -1;
        CPU_ZERO(&cpu[y].mask);
        cpu[y].load = 0;
    }

    CPU_ZERO (&mask);
    CPU_SET ((mCpu = sched_getcpu()), &mask);
    pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &mask);

    sprintf(c, "head -%d /proc/cpuinfo | tail -1\0", (mCpu * 27 + 12));
    FILE *cc = popen(c, "r");
    fscanf(cc, "core id         : %d", &mCore);
    printf("bounding %d %d\n", mCpu, mCore);

    cpu[mCore].load = 1;
    CPU_SET(mCpu, &cpu[mCore].mask);
    double a = boo.a;
    double b = boo.b;
    double result = 0;

    pthread_t threads[procNum + 1];
    int processor, coreId;
    int res = -1;
    borders *bo = malloc(sizeof(borders) * n);
    int loadCpu = 1; //per cpu
    int loadCore;
    if(n<procNum) loadCore=1;
    else loadCore=2;
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    int i = 0;
    bo[0].a = a;
    bo[0].b = a + (b - a) / n;

    while ((res = fscanf(cpuinfo_file, "processor : %d\ncore id : %d\n", &processor, &coreId)) == 2) {
        cpu[coreId].id = processor;
        CPU_SET(processor, &cpu[coreId].mask);
    }

    if (res != EOF) {
        perror("fscanf #1");
        exit(-1);
    }


    for(int w=0; w<=coreIdMax; w++) {
        if (cpu[w].id == -1) continue;
        while (cpu[w].load < loadCore && i<n-1) {
            bo[i].a = a + (b - a) / n * i;
            bo[i].b = a + (b - a) / n * (i + 1);
            pthread_attr_setaffinity_np(&attr, sizeof(cpu_set_t), &cpu[w].mask);
            if ((pthread_create(&threads[i], &attr, threadFunc, &bo[i])) != 0) {
                printf("err creating thread %d", errno);
                return 0;
            }
            printf("starting %dth process #%lu on core %d| current load %d loadCore %d\n",
                   i, threads[i], w, cpu[w].load, loadCore);
            cpu[w].load++;
            i++;
        }
    }
    errno = 0;
    bo[i].a = a + (b - a) / n * i;
    bo[i].b = a + (b - a) / n * (i+1);
    result=result+*((double*) threadFunc(&bo[n-1]));
    double* ret[n+1];
    for(int i=n-2; i>=0 && n>1; i--){
        if(!threads[i]) continue;
        pthread_join(threads[i], (void**) &ret[i]);
        result+=*ret[i];
        free(ret[i]);
    }  /*Wait until thread is finished */
    printf("%e\n", result);
    return result;
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


int main(int argc, char** argv) {
    printf("Hello, World!\n");
    //int udpFd
    int n=input(argc, argv);
    struct sockaddr_in recvAddr = {
            .sin_family=AF_INET,
            .sin_port=htons(UDPPORT),
            .sin_addr.s_addr=htonl(INADDR_ANY)
    };
    struct sockaddr_in tcpAddr = {
            .sin_family=AF_INET,
            .sin_port=0,
            .sin_addr.s_addr=htonl(INADDR_ANY)
    };
    int rbuf = getpid();
    int ovl = 1;
    int udpFd;
    udpFd= socket(PF_INET, SOCK_DGRAM, 0);
    setsockopt(udpFd, SOL_SOCKET, SO_REUSEADDR, &ovl, sizeof(ovl));
    if(bind(udpFd, &recvAddr, sizeof(recvAddr))){
        printf("bind error try another port");
        return -3;
    }
    //setsockopt(udpFd, SOL_SOCKET, SO_BROADCAST, &ovl, sizeof(ovl));
    printf("pid %d", rbuf);
    //double buf = 0;
    msg a; //recieving adress
    int status = -1;
    //printf("send %lu\n", htonl(udpAddr.sin_addr.s_addr));
    //struct sockaddr_in recvAddr;//сюда пишем адрес
    unsigned int recvAddrLen = sizeof(recvAddr);
    //unsigned int udpAddrLen = sizeof(udpAddr);
    struct timeval tv;
    double b=-1;

    if(recvfrom(udpFd, &b, sizeof(b), MSG_WAITALL,  &recvAddr, &recvAddrLen)<0) return -2;
   // setsockopt(udpFd.fd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(tv));
    int tcpFd = socket(PF_INET, SOCK_STREAM, 0);
    //tcpAddr.sin_port=recvAddr.sin_port;
    if(bind(tcpFd, (struct sockaddr*) &tcpAddr, sizeof(tcpAddr))){
        printf("bind error %d", errno);
        //return -3;
    }
    unsigned int tcpAddrLen = sizeof(tcpAddr);
    getsockname(tcpFd, &tcpAddr, &tcpAddrLen);
    a.tcpAddr=tcpAddr;
    //a.tcpAddr.sin_port=tcpAddr.sin_port;
    //tcpAddr.sin_addr=recvAddr.sin_addr;

    if(sendto(udpFd, &a, sizeof(msg), MSG_DONTWAIT, (struct sockaddr*) &recvAddr, sizeof(recvAddr))<0){
        printf("msgsnd error\n");
        return -3;
    }
    printf("send %lu\n", htonl(recvAddr.sin_addr.s_addr));
    /*if(status<0){
        printf("error connecting to client\n");
        return 0;
    }*/
    printf("send-rcv handshake\n");
    printf("waiting for accept \n");
    int sk=-1;
    int o=1;
    setsockopt(tcpFd, SOL_SOCKET, SO_KEEPALIVE, &o, sizeof(o));
    //a.tcpAddr.sin_addr=recvAddr.sin_addr;
    //bind(tcpFd, &recvAddr, sizeof(recvAddr));
    tv.tv_sec = 5;
    tv.tv_usec = 0;
    setsockopt(tcpFd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(tv));

    listen(tcpFd, 256);
    if((sk=accept(tcpFd, NULL, NULL))<0) return 0;

    /*for (int k=0; (o=connect(tcpFd, (struct sockaddr*) &a.tcpAddr, sizeof(a.tcpAddr))==-1 && k<1000); k++){
      //  tv.tv_sec = 1;
      //  tv.tv_usec = 0;
    }*/
    printf("%d\n", sk);
    printf("tcp handshake %d\n", sk);
    borders bo;
    read(sk, &bo, sizeof(bo));
    printf("%f %f\n", bo.a, bo.b);
    double res=integrate(bo, n);
    write(sk, &res, sizeof(res));
    close(sk);
    return 0;
}



