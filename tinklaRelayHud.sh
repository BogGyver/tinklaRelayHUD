#!/bin/bash
sudo systemctl isolate multi-user.target
export QT_QPA_PLATFORM=linuxfb
export QT_QPA_FB_TSLIB=1
export TSLIB_TSDEVICE=/dev/input/mice
/home/ubuntu/tinklaRelayHUD/tinklaRelayHUD /sys/class/backlight/rpi_backlight/brightness
