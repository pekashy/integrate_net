#include "headers.h"
#include <sys/poll.h>
#include <assert.h>


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
    int procNum = get_nprocs();
    int coreNum = 0;
    int coreIdMax = -1;
    const char cheatcode[] = "fgrep -e 'processor' -e 'core id' /proc/cpuinfo";
    FILE *cpuinfo_file = popen(cheatcode, "r");
    FILE *crs = popen("grep 'core id' /proc/cpuinfo | grep -Eo '[0-9]{1,4}' | sort -rn | head -n 1",
                      "r"); //getting maximum cpuId    //sched_setscheduler(pthread_self(), SCHED_FIFO, NULL);
    assert(cpuinfo_file);
    fscanf(crs, "%d", &coreIdMax);
    if (coreIdMax < 0) {
        printf("core num parsing error");
        exit(-1);
    }
    core *cpu = calloc(coreIdMax + 1, sizeof(core));
    for (int y = 0; y < coreIdMax + 1; y++) {
        cpu[y].id = -1;
        CPU_ZERO(&cpu[y].mask);
        cpu[y].load = 0;
    }
    FILE* prcslst=popen("grep 'core id' /proc/cpuinfo | grep -Eo '[0-9]{1,4}'","r");
    FILE* crslst=popen("grep 'core id' /proc/cpuinfo | grep -Eo '[0-9]{1,4}'","r");
    char prcid[4];
    char crid[4];
    int processor, coreId;
    while(fgets(prcid, 4, (FILE*) prcslst) && fgets(crid, 4, (FILE*) crslst)){
        printf("%d %d\n", atoi(crid), atoi(prcid));
        int processor=atoi(prcid), coreId=atoi(crid);
        if (cpu[coreId].id == -1) coreNum++;
        cpu[coreId].id = processor;
        CPU_SET(processor, &cpu[coreId].mask);
    }

    if(n>procNum) n=procNum;
    if (!n || n < 1) return -1;
    double a = boo.a;
    double b = boo.b;
    double result = 0;
    pthread_t thre[procNum + 1];
    pthread_t threads[n + 1];
    borders *bo = calloc(n, sizeof(borders));
    borders *bb = calloc(abs(n - procNum) + 1, sizeof(borders));
    printf("%d", abs((n - procNum)));
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    int minLoadCore = procNum / coreNum;
    int k = n % coreNum;
    int r = k;
    int i=0, t = 0;
    for (int w = 0; w <= coreIdMax; w++) {
        if (cpu[w].id == -1) continue;
        cpu[w].loadCore = n / coreNum + 1 * (r > 0);
        cpu[w].trashLoadCore = (minLoadCore - cpu[w].loadCore) * ((minLoadCore - cpu[w].loadCore) > 0);
        r--;
        for (int tr = 0; tr < cpu[w].trashLoadCore+cpu[w].trashLoadCore*(cpu[w].loadCore==0 && w!=coreIdMax); tr++) {
            pthread_attr_setaffinity_np(&attr, sizeof(cpu_set_t), &cpu[w].mask);
            bb[t].a = a;
            bb[t].b = b*100.0;

            if ((pthread_create(&thre[t], &attr, threadFunc, &bb[t])) != 0) {
                printf("err creating thread %d", errno);
                return 0;
            }
            t++;
        }

    }
    for(int w=0; w<=coreIdMax; w++) {
        if (cpu[w].id == -1) continue;
        while (cpu[w].load < cpu[w].loadCore) {
            bo[i].a = a + (b - a) / n * i;
            bo[i].b = a + (b - a) / n * (i + 1);
            pthread_attr_setaffinity_np(&attr, sizeof(cpu_set_t), &cpu[w].mask);
            if ((pthread_create(&threads[i], &attr, threadFunc, &bo[i])) != 0) {
                printf("err creating thread %d", errno);
                return 0;
            }
            i++;
            cpu[w].load++;
        }
    }
    double* ret[n+1];
    result=0;
    for(int i=n-1; i>=0; i--){
        if(!threads[i]) continue;
        pthread_join(threads[i], (void**) &ret[i]);
        result+=*ret[i];
        free(ret[i]);
    }
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
    printf("pid %d", rbuf);
    msg a; //recieving adress
    unsigned int recvAddrLen = sizeof(recvAddr);
    struct timeval tv;
    int b=-1;
    if(recvfrom(udpFd, &b, sizeof(b), MSG_WAITALL,  &recvAddr, &recvAddrLen)<0) return -2;
    int tcpFd = socket(PF_INET, SOCK_STREAM, 0);
    if(bind(tcpFd, (struct sockaddr*) &tcpAddr, sizeof(tcpAddr))){
        printf("bind error %d", errno);
        //return -3;
    }
    unsigned int tcpAddrLen = sizeof(tcpAddr);
    getsockname(tcpFd, &tcpAddr, &tcpAddrLen);
    a.tcpAddr=tcpAddr;
    a.threadNum=n;

    if(sendto(udpFd, &a, sizeof(msg), MSG_DONTWAIT, (struct sockaddr*) &recvAddr, sizeof(recvAddr))<0){
        printf("msgsnd error\n");
        return -3;
    }
    printf("send-rcv handshake\n");
    printf("waiting for accept \n");
    int sk=-1;
    int o=1;
    setsockopt(tcpFd, SOL_SOCKET, SO_KEEPALIVE, &o, sizeof(o));
    tv.tv_sec = 5;
    tv.tv_usec = 0;
    setsockopt(tcpFd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(tv));

    listen(tcpFd, 256);
    if((sk=accept(tcpFd, NULL, NULL))<0) return 0;
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



