#!/bin/sh
# 将已校验的目标架构程序和服务配置装入离线 rootfs。
set -eu
if [ "$#" -ne 3 ] || [ ! -d "$1" ] || [ ! -x "$2" ] || [ ! -x "$3" ]; then
    echo "Usage: stage_media_services.sh ROOTFS MEDIAMTX_BINARY RCLONE_BINARY" >&2
    exit 2
fi
deploy_dir=$(CDPATH='' cd -- "$(dirname -- "$0")/.." && pwd)
rootfs=$(realpath -- "$1")
if [ "$rootfs" = / ]; then
    echo "Expected an offline rootfs" >&2
    exit 2
fi
install -Dm755 "$2" "$rootfs/usr/local/bin/mediamtx"
install -Dm755 "$3" "$rootfs/usr/local/bin/rclone"
install -Dm755 "$deploy_dir/scripts/upload_completed_task.sh" "$rootfs/usr/local/bin/capture-upload"
install -Dm644 "$deploy_dir/mediamtx/mediamtx.yml" "$rootfs/etc/capture-system/mediamtx.yml"
unit_dir="$rootfs/etc/systemd/system"
install -Dm644 "$deploy_dir/systemd/capture-preview.service" "$unit_dir/capture-preview.service"
mkdir -p "$unit_dir/multi-user.target.wants"
ln -sfn ../capture-preview.service "$unit_dir/multi-user.target.wants/capture-preview.service"
