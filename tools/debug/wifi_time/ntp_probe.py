#!/usr/bin/env python3
"""Read-only NTPv4 relative-clock probe; never sets system time."""
import argparse
import csv
import socket
import struct
import sys
import time

EPOCH = 2208988800
def encode(ns):
    sec, sub = divmod(ns, 1_000_000_000)
    return struct.pack('!II', (sec + EPOCH) & 0xffffffff, (sub << 32) // 1_000_000_000)
def decode(data):
    sec, frac = struct.unpack('!II', data)
    return (sec - EPOCH) * 1_000_000_000 + ((frac * 1_000_000_000) >> 32)
def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('server')
    parser.add_argument('--count', type=int, default=100)
    parser.add_argument('--interval', type=float, default=.1)
    args = parser.parse_args()
    if not 1 <= args.count <= 100000 or not .05 <= args.interval <= 60:
        parser.error('count must be 1..100000, interval .05..60 seconds')
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.connect((args.server, 123))
    sock.settimeout(1)
    writer = csv.writer(sys.stdout)
    writer.writerow(['sample','local_utc_ns','status','server_minus_client_us','round_trip_us','half_round_trip_us','stratum'])
    for i in range(args.count):
        start = time.monotonic_ns()
        t1 = time.time_ns()
        stamp = encode(t1)
        packet = bytes([0x23]) + bytes(39) + stamp
        try:
            sock.send(packet)
            data = sock.recv(512)
            t4 = time.time_ns()
            end = time.monotonic_ns()
            if len(data) < 48 or data[0] & 7 != 4 or data[24:32] != stamp:
                raise ValueError('invalid or stale NTP response')
            if data[0] >> 6 == 3 or not 1 <= data[1] <= 15:
                raise ValueError('server unsynchronised or rate-limited')
            if abs((t4-t1)-(end-start)) > 1_000_000:
                raise ValueError('local wall clock changed during exchange')
            t2, t3 = decode(data[32:40]), decode(data[40:48])
            delay = (t4-t1)-(t3-t2)
            if delay < 0 or t3 < t2:
                raise ValueError('invalid timestamp ordering')
            offset = ((t2-t1)+(t3-t4))/2
            writer.writerow([i,(t1+t4)//2,'ok',f'{offset/1000:.3f}',f'{delay/1000:.3f}',f'{delay/2000:.3f}',data[1]])
        except (OSError,ValueError) as exc:
            writer.writerow([i,t1,str(exc),'','','',''])
        sys.stdout.flush()
        elapsed = (time.monotonic_ns()-start)/1e9
        if elapsed < args.interval:
            time.sleep(args.interval-elapsed)
    sock.close()
if __name__ == '__main__':
    main()
