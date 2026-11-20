#include "imu_manager/imu_sample.h"

#include <math.h>

bool imu_sample_is_valid(const struct imu_sample *sample)
{
    if (sample == 0) {
        return false;
    }

    for (unsigned int axis = 0; axis < 3; ++axis) {
        if (!isfinite(sample->acceleration_m_s2[axis]) ||
            !isfinite(sample->angular_velocity_rad_s[axis])) {
            return false;
        }
    }

    return true;
}
