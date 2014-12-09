#!/bin/bash
# create new file system
dd if=/dev/zero of=hmwk6.fs bs=1M count=2
sudo losetup /dev/loop0 hmwk6.fs
sudo ./userspace/e2fsprogs/misc/mke2fs -I 256 -t ext3 -L w4118.hmwk6 /dev/loop0
sudo losetup -d /dev/loop0

