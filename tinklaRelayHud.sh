#!/bin/bash
sudo systemctl isolate multi-user.target
export QT_QPA_PLATFORM=linuxfb
/home/ubuntu/tinklaRelayHUD/tinklaRelayHUD /sys/class/backlight/rpi_backlight/brightness
