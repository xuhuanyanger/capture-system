# capture-system

RK3576 多视频、IMU 与音频采集工程。当前为目录骨架，尚无可运行程序。Git 已初始化，主分支为 main。

## 目录

```text
capture-system/
├── src/             # C/C++ 实现，按业务模块划分
├── include/         # 对外头文件，与 src 模块同名
├── web/             # 手机网页
├── config/          # 默认配置、结构定义和配置示例
├── tests/           # 单元、集成、整机和性能测试
├── tools/           # 自检、调试、导出和回放工具
├── deploy/          # 服务、权限规则、安装与打包
├── cmake/           # 构建辅助和交叉工具链
├── docs/            # 架构、接口、数据格式和测试说明
├── .github/         # 预留 CI 配置
├── CMakeLists.txt
└── .gitignore
```

## 源码模块

| 模块 | 职责 |
|---|---|
| capture_manager | 任务总控与状态机 |
| camera_manager | 五路 V4L2 采集 |
| imu_manager | IMU 与 FIFO 读取 |
| audio_manager | ALSA 音频采集 |
| sync_manager | 同步控制和事件采集 |
| aligner | 时钟映射与数据关联 |
| encoder | MPP 硬件编码 |
| storage_manager | 封装、分段、落盘与恢复 |
| streaming | 预览媒体与连接 |
| api | HTTP/WebSocket 控制和状态 |
| upload_manager | 上传队列、续传和校验 |
| health_monitor | 健康指标、异常检测与上报 |
| common | 公共类型、日志、错误和基础能力 |

新增模块时同步添加 src、include、tests/unit 中的对应目录，并在 src/CMakeLists.txt 登记。
共享头文件放 include；模块私有头文件可放在对应 src 模块中。未实现功能不提前创建细分目录。

## 配置与构建

图像目标为五路 1920×1200、30 fps；硬件节点、IMU 配置、码率与同步目标待确认。
`config/examples/capture.example.json` 为配置设计示例，null 表示未确定，当前无配置加载程序。

```sh
cmake -S . -B build
cmake --build build
```

当前仅验证骨架，不生成程序；实际开发时启用 C/C++ 并接入 SDK 依赖。
录像、日志、真实凭据、编译产物不提交；运行数据可放忽略的 data、runtime、logs 目录。
内核、设备树和板厂驱动在 SDK 仓库中维护。
