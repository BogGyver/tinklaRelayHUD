#!/bin/bash
sudo systemctl isolate multi-user.target
export QT_QPA_PLATFORM=linuxfb
./tinklaRelayHUD /sys/class/backlight/rpi_backlight/brightness
