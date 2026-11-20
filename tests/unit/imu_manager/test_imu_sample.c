#include "imu_manager/imu_sample.h"

#include <math.h>
#include <stdio.h>

static int check(int condition, const char *description)
{
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", description);
        return 1;
    }

    printf("PASS: %s\n", description);
    return 0;
}

int main(void)
{
    struct imu_sample sample = {
        .timestamp_ns = 123456789,
        .acceleration_m_s2 = {0.0, 0.0, 9.80665},
        .angular_velocity_rad_s = {0.01, -0.02, 0.03},
    };
    int failures = 0;

    failures += check(imu_sample_is_valid(&sample), "finite sample is accepted");
    failures += check(!imu_sample_is_valid(0), "null sample is rejected");

    sample.acceleration_m_s2[1] = NAN;
    failures += check(!imu_sample_is_valid(&sample), "NaN acceleration is rejected");

    sample.acceleration_m_s2[1] = 0.0;
    sample.angular_velocity_rad_s[2] = INFINITY;
    failures += check(!imu_sample_is_valid(&sample), "infinite angular velocity is rejected");

    return failures == 0 ? 0 : 1;
}
