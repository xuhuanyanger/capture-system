#ifndef IMU_MANAGER_IMU_SAMPLE_H
#define IMU_MANAGER_IMU_SAMPLE_H

#include <stdbool.h>
#include <stdint.h>
#include "common/timestamp.h"

struct imu_sample {
    /* Legacy host timestamp; use time.flags to interpret new metadata. */
    uint64_t timestamp_ns;
    double acceleration_m_s2[3];
    double angular_velocity_rad_s[3];
    struct capture_timestamp time;
};

bool imu_sample_is_valid(const struct imu_sample *sample);

#endif
