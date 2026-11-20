# 模块接口契约

公共时间模型见 include/common/timestamp.h，错误类型见 include/common/status.h。每个实际模块在实现前定义 init/start/stop/destroy、输入输出、线程安全、所有权、超时、取消及状态事件。当前业务 API 尚未实现。

init 获取资源，start 启动处理；stop 接收截止时间并可重复调用；destroy 仅在 worker 和缓冲引用释放后执行。回调不得在持有业务锁时调用外部模块。异步交付必须明确复制或转移所有权；禁止传递栈对象给异步消费者。

事件包含版本、event_id、task_id、stream_id、单调时间、严重性、错误码和上下文；高频重复事件可聚合，但须保留次数及首次/末次时间。状态查询返回快照版本，订阅端允许通过快照恢复丢失事件。

控制接口规划：GET /v1/status、GET /v1/capabilities、POST /v1/tasks、GET /v1/tasks/{id}、POST /v1/tasks/{id}/stop、GET /v1/events。创建请求携带 request_id 和配置，校验失败返回字段级原因，状态冲突返回 409；异步接收返回 202 和任务查询地址。能力接口仅报告已探测能力。WebSocket 需有界发送队列与重连游标，慢消费者不得阻塞总控。

API 与蓝牙使用同一命令模型及授权策略。蓝牙仅做发现、绑定和控制接入，不提供精密同步；硬件、手机端形态和协议未确认前不实现假定协议。配置凭据、绑定密钥和令牌不写入日志或任务快照。当前 MediaMTX 配置允许局域网匿名观看，仅适用于明确接受该策略的部署；非可信网络启用认证与传输保护。
