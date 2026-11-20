add_library(imu_sample STATIC imu_sample.c)
target_include_directories(imu_sample PUBLIC "${PROJECT_SOURCE_DIR}/include")
target_compile_features(imu_sample PUBLIC c_std_11)
