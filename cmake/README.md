# 构建与交叉编译

使用 CMakePresets.json 的 host-test 预设执行主机检查。产物位于各构建目录的 bin/ 与 lib/，避免主机和交叉编译产物相互覆盖。

仓库工具链配置不写死本机 SDK 路径：

```sh
cmake -S . -B out/build-arm \
  -DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/arm-linux-gnueabihf.cmake \
  -DCAPTURE_TOOLCHAIN_ROOT=/path/to/gcc-arm-toolchain \
  -DBUILD_TESTING=OFF -DBUILD_WIFI_TIME_TOOLS=ON
cmake --build out/build-arm
```

该配置针对 arm-none-linux-gnueabihf 用户空间；RK3576 的实际用户空间位宽、ABI、libc 与 SDK 必须匹配。不能把该工具链视为所有 RK3576 镜像的通用选择。交叉编译不在主机执行目标二进制。工具链发行目录不应作为项目源码提交。
