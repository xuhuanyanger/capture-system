# capture-system

基于 RK3576 的五路视频、IMU 与音频同步采集工程，目标视频规格为 1920×1200、30 fps。

当前处于开发初期：已提供 IMU 样本校验、Wi-Fi 校时诊断工具、配置校验工具，以及预览和上传部署脚本；采集主服务尚未实现。

## 构建与测试

需要 C 编译器、CMake 3.20+ 和 Python 3。在仓库根目录执行：

```sh
cmake --preset host-test
cmake --build --preset host-test --parallel 2
ctest --preset host-test
```

产物位于 `out/build-host-test/bin/` 和 `out/build-host-test/lib/`。交叉编译见 [构建说明](cmake/README.md)。

## 配置

配置示例为 [capture.example.json](config/examples/capture.example.json)，其中 `null` 表示待确认参数。

```sh
python3 tools/selfcheck/validate_config.py config/examples/capture.example.json --allow-incomplete
```

检查实际配置时去掉 `--allow-incomplete`，拒绝未确定参数。配置校验不替代硬件能力与同步精度验收。

## 目录结构

```text
capture-system/
├── .github/workflows/        # 主机构建与测试 CI
├── cmake/toolchains/         # 交叉编译配置
├── config/
│   ├── calibration/          # 标定资料
│   ├── defaults/             # 默认配置
│   ├── devices/              # 设备映射
│   ├── examples/             # 采集与 Chrony 配置示例
│   └── schemas/              # 配置结构定义
├── deploy/
│   ├── mediamtx/             # 预览服务配置
│   ├── packaging/            # 打包规则预留
│   ├── scripts/              # 部署与上传脚本
│   ├── systemd/              # 服务单元
│   └── udev/                 # 设备规则预留
├── docs/
│   ├── architecture/         # 模块边界、运行架构、时间模型与状态机
│   ├── data_format/          # 数据格式与恢复协议
│   ├── development/          # 开发约定
│   ├── errors/               # 错误处理
│   ├── hardware/             # 硬件确认清单
│   ├── interfaces/           # 接口契约
│   └── testing/              # 验收要求
├── include/                  # 按模块组织的公开头文件
├── src/
│   ├── aligner/              # 时间映射与数据关联
│   ├── api/                  # 控制与状态接口
│   ├── audio_manager/        # 音频采集
│   ├── camera_manager/       # 相机采集
│   ├── capture_manager/      # 任务状态与模块编排
│   ├── common/               # 公共类型与基础能力
│   ├── encoder/              # 硬件编码
│   ├── health_monitor/       # 健康监控
│   ├── imu_manager/          # IMU 样本处理与采集
│   ├── storage_manager/      # 封装、分段与落盘
│   ├── streaming/            # 实时预览
│   ├── sync_manager/         # 同步源与时钟状态
│   └── upload_manager/       # 任务上传
├── tests/
│   ├── fault_injection/      # 故障注入预留
│   ├── fixtures/             # 测试样例
│   ├── integration/          # 集成测试预留
│   ├── performance/          # 性能测试预留
│   ├── system/               # 整机测试预留
│   └── unit/                 # 按模块组织的单元测试
├── tools/
│   ├── benchmark/            # 性能工具预留
│   ├── debug/wifi_time/      # Wi-Fi 校时诊断
│   ├── export/               # 导出工具预留
│   ├── playback/             # 回放工具预留
│   └── selfcheck/            # 配置自检
├── web/                      # 网页控制端预留
├── CMakeLists.txt            # 构建入口
└── CMakePresets.json         # 构建与测试预设
```

`src/`、`include/`、`tests/unit/` 下的同名目录对应同一模块；多数业务模块仍为骨架。

## 文档

- 架构：[模块边界](docs/architecture/module-boundaries.md)、[运行架构](docs/architecture/runtime.md)、[时间模型](docs/architecture/time-model.md)、[任务状态机](docs/architecture/task-lifecycle.md)。
- 契约：[模块接口](docs/interfaces/README.md)、[数据格式与恢复](docs/data_format/README.md)、[配置说明](config/README.md)。
- 验证：[硬件清单](docs/hardware/README.md)、[测试验收](docs/testing/README.md)。
- 开发：[实施路线](docs/architecture/roadmap.md)、[工具使用](tools/README.md)、[预览与上传部署](deploy/README.md)。
