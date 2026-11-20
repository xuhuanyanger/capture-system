# 配置

schemas/capture.schema.json 定义 v1 设计结构；examples/capture.example.json 中 null 表示待确认值。defaults/ 不提供臆造的生产参数。生产配置必须无 null，并经过设备能力和任务资源检查。

```sh
python3 tools/selfcheck/validate_config.py config/examples/capture.example.json --allow-incomplete
```

默认严格模式拒绝未确定值、未知字段、重复 JSON 键、重复设备映射、非正采样率及非绝对存储路径。校验器仅处理本仓库 schema 使用的约束子集；schema 增加关键字时须同步更新校验器和测试。配置结构校验不代表硬件支持目标参数。

真实凭据、设备私有标定不提交。任务开始时保存脱敏配置快照及摘要；标定引用须包含版本和摘要。运行时配置加载、迁移及硬件能力探测待实现。
