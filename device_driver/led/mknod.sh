#!/bin/sh

MODULE="led_device"
MAJOR=$(awk "\$2==\"$MODULE\" {print \$1}" /proc/devices)

mknod /dev/${MODULE}0 c $MAJOR 0
sudo chmod 666 /dev/${MODULE}0

mknod /dev/${MODULE}1 c $MAJOR 1
sudo chmod 666 /dev/${MODULE}1