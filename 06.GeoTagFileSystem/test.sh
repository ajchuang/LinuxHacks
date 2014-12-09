#!/bin/bash

# create loopback link
adb shell ln -s /dev/block/loop0 /dev/loop0
adb shell ln -s /dev/block/loop1 /dev/loop1
adb shell ln -s /dev/block/loop2 /dev/loop2
adb shell ln -s /dev/block/loop3 /dev/loop3
adb shell ln -s /dev/block/loop4 /dev/loop4
adb shell ln -s /dev/block/loop5 /dev/loop5
adb shell ln -s /dev/block/loop6 /dev/loop6
adb shell ln -s /dev/block/loop7 /dev/loop7

# upload file system file
adb shell rm /data/misc/hmwk6.fs
adb push hmwk6.fs /data/misc
adb shell rmdir /data/misc/hmwk6
adb shell mkdir /data/misc/hmwk6

# mount device, start gps daemon
adb shell mount -o loop -t ext3 /data/misc/hmwk6.fs /data/misc/hmwk6
#adb shell mount -o loop=/dev/loop0 -t ext3 /data/misc/hmwk6.fs /data/misc/hmwk6
#adb shell /data/misc/gpsd 1>/dev/null 2>/dev/null

#adb shell umount /data/misc/hmwk6
#adb pull /data/misc/hmwk6.fs

#adb shell mkdir /data/misc/hmwk6/test3
#adb shell cat proc/kmsg
#adb shell /data/misc/file_loc /data/misc/hmwk6/test3
