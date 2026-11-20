cmake_minimum_required(VERSION 3.16)
project(capture_system LANGUAGES C)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")

include(CTest)
option(BUILD_WIFI_TIME_TOOLS "Build Linux Wi-Fi timing diagnostics" OFF)
option(BUILD_IMU_SAMPLE_TESTS "Build IMU sample tests (legacy option)" OFF)
if(BUILD_IMU_SAMPLE_TESTS)
    enable_testing()
endif()
if(BUILD_WIFI_TIME_TOOLS)
    add_subdirectory(tools/debug/wifi_time)
endif()
add_subdirectory(src)
if(BUILD_TESTING OR BUILD_IMU_SAMPLE_TESTS)
    add_subdirectory(tests/unit/imu_manager)
    find_package(Python3 COMPONENTS Interpreter REQUIRED)
    add_test(NAME config_example COMMAND "${Python3_EXECUTABLE}"
        "${PROJECT_SOURCE_DIR}/tools/selfcheck/validate_config.py"
        "${PROJECT_SOURCE_DIR}/config/examples/capture.example.json" --allow-incomplete)
    add_test(NAME config_validator COMMAND "${Python3_EXECUTABLE}"
        "${PROJECT_SOURCE_DIR}/tests/unit/common/test_config_validation.py")
endif()
