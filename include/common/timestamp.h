#ifndef CAPTURE_COMMON_TIMESTAMP_H
#define CAPTURE_COMMON_TIMESTAMP_H

#include <stdint.h>

enum capture_clock_domain {
    CAPTURE_CLOCK_UNKNOWN = 0,
    CAPTURE_CLOCK_HOST_MONOTONIC = 1,
    CAPTURE_CLOCK_SENSOR = 2,
    CAPTURE_CLOCK_AUDIO = 3,
    CAPTURE_CLOCK_TRIGGER = 4
};

enum capture_timestamp_flags {
    CAPTURE_TIME_RAW_VALID = 1u << 0,
    CAPTURE_TIME_COMMON_VALID = 1u << 1,
    CAPTURE_TIME_ERROR_VALID = 1u << 2,
    CAPTURE_TIME_SEQUENCE_VALID = 1u << 3
};

/* In-memory metadata, never serialize by dumping this struct. */
struct capture_timestamp {
    uint64_t raw_ticks;
    uint64_t tick_hz;
    uint64_t common_ns;
    uint64_t error_bound_ns;
    uint64_t sequence;
    uint32_t source_id;
    uint32_t epoch;
    uint32_t mapping_version;
    enum capture_clock_domain clock_domain;
    uint32_t flags;
};

#endif
