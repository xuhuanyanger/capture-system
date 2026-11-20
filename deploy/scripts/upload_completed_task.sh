#!/bin/sh
# 调用方在关闭所有文件后原子创建 .complete，保证重试读取同一份任务。
set -eu
: "${CAPTURE_UPLOAD_REMOTE:?set rclone destination remote:path}"
if [ "$#" -ne 1 ] || [ ! -d "$1" ] || [ ! -f "$1/.complete" ]; then
    echo "Expected one completed task directory containing .complete" >&2
    exit 2
fi
task_path=$(realpath -- "$1")
task_name=$(basename -- "$task_path")
case "$task_name" in
    ''|/|*[!a-zA-Z0-9_.-]*) echo "Invalid task directory name" >&2; exit 2 ;;
esac
exec rclone copy --exclude '/.complete' -- "$task_path" "${CAPTURE_UPLOAD_REMOTE%/}/$task_name"
