# 主机 CI

workflows/ci.yml 在 push 和 pull_request 时编译主机工具及 IMU 库，运行 IMU 与配置验证测试。只授予 contents:read 权限，不部署、不上传任务数据。板端性能和同步验收须另行执行，见 docs/testing/README.md。
