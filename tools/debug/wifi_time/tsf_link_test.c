/* Four hardware-counter timestamps over WiFi. Does not change either counter.
 * Diagnostic protocol for the two little-endian ARM64 boards, not a public API. */
#define main tsf_read_tool_main
#include "tsf_read.c"
#undef main
#include <arpa/inet.h>
#include <sys/time.h>
struct packet { uint64_t magic, seq, t1, t2, t3, w1, w2, w3; };
#define MAGIC UINT64_C(0x5453465445535431)
static uint64_t read_tsf(uint32_t base, uint64_t *window) {
    uint64_t t,b,a;
    if (sample(base,&t,&b,&a)) {perror("sample");exit(1);}
    *window=(a-b+999)/1000;
    return t;
}
int main(int argc,char **argv) {
    if(argc<3 || argc>4 || (strcmp(argv[1],"server") && strcmp(argv[1],"client"))) {
        fprintf(stderr,"Usage: %s server|client peer_ipv4 [count_or_duration=100|20]\n",argv[0]);return 2;
    }
    int server=!strcmp(argv[1],"server");
    char *end;long n=server?20:100;
    if(argc==4){n=strtol(argv[3],&end,10);if(*end || n<1 || n>(server?60:1000))return 2;}
    strcpy(iface,"wlan0");sockfd=socket(AF_INET,SOCK_DGRAM,0);
    if(sockfd<0){perror("ioctl socket");return 1;}
    uint32_t base=discover_d11();
    int fd=socket(AF_INET,SOCK_DGRAM,0);if(fd<0){perror("udp socket");return 1;}
    struct timeval to={.tv_sec=1};setsockopt(fd,SOL_SOCKET,SO_RCVTIMEO,&to,sizeof(to));
    struct sockaddr_in addr={.sin_family=AF_INET,.sin_port=htons(47992)};
    if(inet_pton(AF_INET,argv[2],&addr.sin_addr)!=1)return 2;
    if(server) {
        if(bind(fd,(void*)&addr,sizeof(addr))){perror("bind");return 1;}
        uint64_t deadline=host_ns()+(uint64_t)n*1000000000ULL;unsigned count=0;
        while(host_ns()<deadline && count<10000) {
            struct packet p;struct sockaddr_in from;socklen_t len=sizeof(from);
            ssize_t r=recvfrom(fd,&p,sizeof(p),0,(void*)&from,&len);
            if(r!=(ssize_t)sizeof(p)||p.magic!=MAGIC)continue;
            if((ntohl(from.sin_addr.s_addr)&0xffffff00U)!=0x0a2a0000U)continue;
            p.t2=read_tsf(base,&p.w2);
            p.t3=read_tsf(base,&p.w3);
            if(sendto(fd,&p,sizeof(p),0,(void*)&from,len)!=(ssize_t)sizeof(p)){perror("sendto");return 1;}
            count++;
        }
        fprintf(stderr,"server_packets=%u\n",count);
    } else {
        if(connect(fd,(void*)&addr,sizeof(addr))){perror("connect");return 1;}
        puts("seq,status,t1_us,t2_us,t3_us,t4_us,offset_ap_minus_sta_us,delay_us,read_windows_sum_us,offset_lower_us,offset_upper_us");
        for(long i=0;i<n;i++) {
            struct packet p={.magic=MAGIC,.seq=(uint64_t)i};
            p.t1=read_tsf(base,&p.w1);uint64_t t1=p.t1;
            if(send(fd,&p,sizeof(p),0)!=(ssize_t)sizeof(p)){perror("send");return 1;}
            ssize_t r=recv(fd,&p,sizeof(p),0);uint64_t t4,w4;
            t4=read_tsf(base,&w4);
            if(r!=(ssize_t)sizeof(p)||p.magic!=MAGIC||p.seq!=(uint64_t)i||p.t1!=t1) {
                printf("%ld,invalid_or_timeout,,,,,,,,,\n",i);fflush(stdout);continue;
            }
            int64_t x=(int64_t)p.t2-(int64_t)p.t1,y=(int64_t)p.t3-(int64_t)t4;
            int64_t delay=(int64_t)(t4-p.t1)-(int64_t)(p.t3-p.t2);
            uint64_t w=p.w1+p.w2+p.w3+w4;
            printf("%ld,%s,%"PRIu64",%"PRIu64",%"PRIu64",%"PRIu64",%.1f,%"PRId64",%"PRIu64",%"PRId64",%"PRId64"\n",
                i,delay>=0?"ok":"negative_delay",p.t1,p.t2,p.t3,t4,(x+y)/2.0,delay,w,
                y-(int64_t)w,x+(int64_t)w);
            fflush(stdout);
            struct timespec pause={.tv_nsec=50000000};nanosleep(&pause,NULL);
        }
    }
    close(fd);close(sockfd);return 0;
}
