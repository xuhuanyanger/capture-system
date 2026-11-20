# 预览与上传部署

在 Linux 构建机上，准备与目标用户空间架构匹配的 MediaMTX v1 和 rclone Linux 程序，按各自发布页校验下载。对使用 systemd 的离线根文件系统执行：

```sh
deploy/scripts/stage_media_services.sh "$ROOTFS" "$MEDIAMTX_BINARY" "$RCLONE_BINARY"
```

脚本覆盖根文件系统中的同名部署文件并启用 capture-preview.service。镜像启动后，可通过 journalctl -u capture-preview 检查配置加载情况。采集端通过 TCP 将浏览器支持的 H.264 码流发布到 rtsp://127.0.0.1:8554/camera1，手机在同一可信局域网打开 http://设备地址:8889/camera1/ 。路径支持 camera1 至 camera5。预览允许匿名读取，需放通 TCP 8889 和 UDP 8189。当前采集骨架尚无推流程序。

为上传进程配置 rclone remote 和凭据（可用 RCLONE_CONFIG 指定配置文件）。任务写入方关闭所有文件后创建 .complete 标记，此后保持目录内容不变。任务目录名使用英文、数字、下划线、点或连字符。上传成功返回 0，失败可用相同参数重试：

```sh
CAPTURE_UPLOAD_REMOTE='archive:capture' capture-upload /data/tasks/session_001
```

目标目录为 archive:capture/session_001；本地标记不上传。采集总控需在任务结束后调用此命令。


## 能力边界

当前上传脚本提供传输入口，不提供持久化队列、应用层远端提交或自动删除。任务写入者须执行 [数据提交协议](../docs/data_format/README.md)，后续 upload_manager 负责清单验证、远端完成确认和退避恢复。当前 .complete 存在性检查不是内容校验。

生产交付必须记录已验证的 MediaMTX/rclone 精确版本与 SHA-256。局域网匿名预览是当前显式配置，其他部署须补充认证策略。capture-service、权限规则及完整安装包尚待实现，不能仅凭预览服务启动判断采集可用。
