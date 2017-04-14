#!/bin/sh
#rm 6291-update-fw.bin
#rm 6291-update-fw.bin.md5
cd /tmp/ && tar -xzf /tmp/mnt/USB-disk-1/6291-update-fw-gz.bin -C /tmp/ && cp 6291-update-fw.bin /tmp/fwupgrade && sysupgrade /tmp/fwupgrade
