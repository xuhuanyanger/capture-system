# 模块职责

- capture_manager 统一决定启停、降级和恢复。
- health_monitor 检测和上报异常，不直接控制其他模块。
- sync_manager 调用驱动控制同步源并收集事件；内核驱动、设备树在 SDK 中维护。
- aligner 保留原始时间戳，建立时钟映射与关联，不掩盖丢帧或同步异常。
- encoder 负责编码，storage_manager 负责封装、分段和落盘。
- streaming 负责实时预览，upload_manager 负责已完成任务上传。
- api 接收控制请求并调用总控，不绕过总控操作设备。
- common 仅提供通用能力，不依赖业务模块。

硬件连接、LMK 器件型号、CAP0 含义与 Trigger ID 规则需在硬件接口文档确认。
