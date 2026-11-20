# 任务数据与提交协议 v1

状态：格式设计基线；封装器和恢复工具尚未实现。

任务目录使用全局唯一 task_id，包含 manifest.json、video/<stream>/<segment>.mp4、index/<stream>.jsonl、imu/<stream>.jsonl、audio/<stream>/<segment>.wav、sync/events.jsonl、sync/mappings.jsonl 和 diagnostics/events.jsonl。大规模 JSONL 可在格式版本升级后替换为二进制，必须提供兼容读取器。

manifest 至少包含 format_version、task_id、设备及主板标识、软件/SDK 版本、配置快照与摘要、标定引用与摘要、各流格式、时钟注册表、开始/结束时间、结束原因、质量摘要和文件列表。文件项包含相对路径、大小与 SHA-256；禁止绝对路径和越出任务目录的路径。

视频逐帧索引记录流 ID、采集序号、原始时间、映射时间和版本、触发 ID、时间有效性、segment、PTS、DTS、time_base 和关键帧标志。编码器输出须能追溯输入帧，不能仅依赖 PTS 推断曝光时间。IMU 保存单位、轴向、原始与校准值语义；音频保存样本数、块时间和 xrun。未知字段使用显式无效状态，不能用零代替缺失。

## 分段与断电恢复

优先评估 fragmented MP4 或短分段方案；具体封装参数须经播放器及断电试验验证。普通 MP4 未完成收尾可能不可读，不能声称支持无损断电恢复。分段从可独立解码的关键帧开始，清单记录各流真实覆盖区间，不要求文件切点机械一致。

## 本地提交顺序

1. 关闭输入并排空，完成分段；对需要保留的数据文件执行 fsync 后关闭。
2. 写 manifest.json.tmp，fsync 后同文件系统原子 rename 为 manifest.json，再同步目录。
3. 仅成功且完整任务创建 .complete.tmp，内容为 manifest 的 SHA-256；fsync 后 rename 为 .complete，再同步目录。失败任务保留清单和故障记录，不创建 .complete。
4. 发布完成标记后任务内容不可变。启动恢复对未提交目录检查分段及索引，输出恢复报告，不伪造成功标记。

目标文件系统的 rename/fsync 行为与掉电保护必须板端验证。

## 上传提交

上传队列持久化 task_id、源清单摘要、目的地、尝试次数、下次重试时间和错误。先校验本地完成标记及清单，上传数据到唯一任务前缀，检查大小与可用的内容校验，再最后发布远端完成记录。远端读者只消费已提交任务。不同后端的哈希能力必须明确，不把 ETag 统一当作 SHA-256。

当前 capture-upload 只是 rclone copy 传输入口，不实现持久化队列、远端事务、端到端校验或完成确认；返回 0 不能替代上述应用层提交。自动删除本地任务必须等待已验证的远端完成状态和保留策略，默认关闭。
