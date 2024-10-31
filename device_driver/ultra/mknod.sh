#!/bin/sh

MODULE="ultra_device"
MAJOR=$(awk "\$2==\"$MODULE\" {print \$1}" /proc/devices)

mknod /dev/$MODULE c $MAJOR 0
sudo chmod 666 /dev/$MODULE