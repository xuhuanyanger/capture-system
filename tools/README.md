# tools

自检、调试、导出、回放及性能测量工具。

## Wi-Fi 校时诊断

从仓库根目录在 Linux 上构建。板端使用与目标系统匹配的交叉编译器。

```sh
cmake -S . -B build -DBUILD_WIFI_TIME_TOOLS=ON
cmake --build build --target tsf-read tsf-link-test
python3 tools/debug/wifi_time/ntp_probe.py --help
```

两个 C 工具位于 build/bin/。在支持对应 Broadcom SDIO 驱动接口的板上运行
`tsf-read wlan0 10 1000`，按 1000 毫秒间隔读取 10 次本地计数器。
tsf-link-test 用于两板链路测试，运行时不传参数可查看调用格式。现有应答端限定 10.42.0.0/24。
本地 TSF 与上游 Beacon 的时间轴可能不同；相机同步精度需用触发事件和实际帧验证。

ntp_probe.py 接收 NTP 服务器地址并输出 CSV，不改系统时钟。
config/examples/chrony 中的两个配置分别适用于 AP 和 STA，样例假定 AP 为 10.42.0.1。
离线 AP 只能提供共同的相对时间参考。


## 配置自检

`python3 tools/selfcheck/validate_config.py CONFIG.json` 严格检查配置字段、类型、必填项、设备映射唯一性及正值约束。设计示例可显式加 `--allow-incomplete`；生产参数禁止使用该开关。校验器仅实现仓库 schema 所使用的 JSON Schema 子集，不是通用 JSON Schema 引擎，也不替代运行时硬件探测。
