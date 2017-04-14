#!/bin/sh
cd /tmp/ && cp /tmp/mnt/USB-disk-1/ota/update.bin /tmp/mnt/USB-disk-1/6291-update-fw-gz.bin && tar -xzf /tmp/mnt/USB-disk-1/6291-update-fw-gz.bin -C /tmp/ && cp 6291-update-fw.bin /tmp/fwupgrade && sysupgrade /tmp/fwupgrade
