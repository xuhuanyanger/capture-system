#define _POSIX_C_SOURCE 200809L
#define _DEFAULT_SOURCE
#include <errno.h>
#include <inttypes.h>
#include <net/if.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>
#include <linux/sockios.h>

/* Native 64-bit ABI from this SDK's wlioctl.h. Never issues a write ioctl. */
struct wl_ioctl {
    uint32_t cmd;
    void *buf;
    uint32_t len;
    uint8_t set, pad[3];
    uint32_t used, needed;
};
struct request { struct wl_ioctl ioc; uint32_t driver; };
_Static_assert(sizeof(struct wl_ioctl) == 32, "Requires Linux ARM64 ABI");
static int sockfd;
static char iface[IFNAMSIZ];

static uint64_t host_ns(void) {
    struct timespec t;
    if (clock_gettime(CLOCK_MONOTONIC_RAW, &t)) { perror("clock_gettime"); exit(1); }
    return (uint64_t)t.tv_sec * 1000000000ULL + t.tv_nsec;
}

static int read_reg(uint32_t address, uint32_t *value) {
    /* IOV_SBREG interprets offset relative to SI_ENUM_BASE, 0x18000000 here. */
    if (address < 0x18000000U || address >= 0x18200000U || (address & 3)) {
        errno = EINVAL; return -1;
    }
    unsigned char buf[256] = {0};
    int32_t param[3] = {4, (int32_t)(address - 0x18000000U), 0};
    memcpy(buf, "sbreg", 6);
    memcpy(buf + 6, param, sizeof(param));
    struct request req = {.ioc = {.cmd = 2, .buf = buf, .len = sizeof(buf)},
                          .driver = 0x00444944};
    struct ifreq ifr = {0};
    memcpy(ifr.ifr_name, iface, strlen(iface) + 1);
    ifr.ifr_data = (void *)&req;
    if (ioctl(sockfd, SIOCDEVPRIVATE, &ifr) < 0) return -1;
    memcpy(value, buf, sizeof(*value));
    return 0;
}

static uint32_t checked_read(uint32_t address) {
    uint32_t value;
    if (read_reg(address, &value)) {
        fprintf(stderr, "read 0x%08x: %s\n", address, strerror(errno)); exit(1);
    }
    return value;
}

static uint32_t discover_d11(void) {
    uint32_t chip = checked_read(0x18000000), erom = checked_read(0x180000fc);
    if ((chip & 0xffff) != 0x4345 || ((chip >> 16) & 0xf) != 9) {
        fprintf(stderr, "Unsupported chip/revision: 0x%08x; validated only on 4345 rev9\n", chip);
        exit(1);
    }
    if (erom < 0x18000000 || erom > 0x181ff000 || (erom & 0xfff)) {
        fprintf(stderr, "Invalid EROM pointer: 0x%08x\n", erom); exit(1);
    }
    /* Match the exact core identity first; then its port-0 slave descriptor.
       Reject newer cores rather than assume their register layout. */
    for (unsigned i = 0; i < 256; i++) {
        uint32_t cia = checked_read(erom + i * 4);
        if (!(cia & 1)) continue;
        if ((cia & 0xf) == 0xf) break;
        if ((cia & 0xf) != 1 || ((cia >> 8) & 0xfff) != 0x812 ||
            (cia >> 20) != 0x4bf) continue;
        uint32_t cib = checked_read(erom + (++i) * 4);
        if ((cib & 0xf) != 1 || (cib >> 24) != 54) {
            fprintf(stderr, "Unsupported D11 revision/descriptor: 0x%08x\n", cib); exit(1);
        }
        for (unsigned j = 1; j <= 8; j++) {
            uint32_t ad = checked_read(erom + (i + j) * 4);
            if ((ad & 1) && (ad & 6) == 0) break;
            if ((ad & 7) == 5 && !(ad & 0xfc8)) {
                uint32_t base = ad & 0xfffff000U;
                if (base < 0x18000000 || base > 0x181ff000) break;
                fprintf(stderr, "chip=4345 rev=9 D11_rev=54 base=0x%08x; source=SDIO_backplane\n", base);
                return base;
            }
        }
        break;
    }
    fprintf(stderr, "No validated D11 port-0 register space found\n"); exit(1);
}

static int sample(uint32_t base, uint64_t *tsf, uint64_t *before, uint64_t *after) {
    for (int tries = 0; tries < 8; tries++) {
        uint32_t h1, low, h2;
        *before = host_ns();
        if (read_reg(base + 0x184, &h1) || read_reg(base + 0x180, &low) ||
            read_reg(base + 0x184, &h2)) return -1;
        *after = host_ns();
        if (h1 == h2) { *tsf = ((uint64_t)h1 << 32) | low; return 0; }
    }
    errno = EAGAIN; return -1;
}

int main(int argc, char **argv) {
    if (argc < 2 || argc > 4 || strlen(argv[1]) >= IFNAMSIZ) {
        fprintf(stderr, "Usage: %s interface [count=1] [interval_ms=1000]\n", argv[0]); return 2;
    }
    char *end;
    long count = 1, ms = 1000;
    if (argc > 2) { count = strtol(argv[2], &end, 10); if (*end || count < 1 || count > 1000000) return 2; }
    if (argc > 3) { ms = strtol(argv[3], &end, 10); if (*end || ms < 1 || ms > 60000) return 2; }
    strcpy(iface, argv[1]);
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) { perror("socket"); return 1; }
    uint32_t base = discover_d11();
    puts("sample,host_before_ns,host_after_ns,host_mid_ns,tsf_us,read_window_us,delta_tsf_us,delta_host_us");
    uint64_t prev_tsf = 0, prev_mid = 0;
    for (long i = 0; i < count; i++) {
        uint64_t tsf, before, after;
        if (sample(base, &tsf, &before, &after)) { perror("TSF read"); close(sockfd); return 1; }
        uint64_t mid = before + (after - before) / 2;
        printf("%ld,%" PRIu64 ",%" PRIu64 ",%" PRIu64 ",%" PRIu64 ",%.3f,%" PRId64 ",%.3f\n",
               i, before, after, mid, tsf, (after-before)/1000.0,
               i ? (int64_t)tsf-(int64_t)prev_tsf : 0, i ? (mid-prev_mid)/1000.0 : 0);
        fflush(stdout);
        if (i && tsf < prev_tsf) fprintf(stderr, "TSF jumped backwards; invalidate time mapping\n");
        prev_tsf = tsf; prev_mid = mid;
        if (i+1 < count) {
            struct timespec delay = {.tv_sec = ms/1000, .tv_nsec = (ms%1000)*1000000};
            while (nanosleep(&delay, &delay) && errno == EINTR) {}
        }
    }
    close(sockfd);
    return 0;
}
