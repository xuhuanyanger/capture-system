# imu_manager

实现放在本目录，对外头文件放在 `include/imu_manager/`，单元测试放在 `tests/unit/imu_manager/`。
当前提供 IMU 样本结构及有限值校验函数；实际传感器驱动、FIFO 读取和量程校准尚未实现。

```sh
cmake -S . -B out/build-imu -DBUILD_IMU_SAMPLE_TESTS=ON
cmake --build out/build-imu
ctest --test-dir out/build-imu --output-on-failure
```

测试可执行文件输出为 `out/build-imu/bin/imu_sample_test`。

样本增加公共 `capture_timestamp` 元数据，支持来源、计数频率、epoch、映射版本和有效标志。原 `timestamp_ns` 暂保留兼容已有源码；新采集实现使用 `time` 字段。结构体布局已改变，不能作为旧二进制 ABI 或直接写盘格式使用。`imu_sample_is_valid` 只校验物理数值有限性，不表示时间有效或标定通过。
