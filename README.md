# tinklaRelayHUD
HUD to work with Tinkla Relay

Download on an rPi with a 800x480 display. If your display is NOT 800x480 or you are using HDMI, please do set the framebuffer size to 800x480.

To automatically start the HUD just edit the crontab as root and add the following line to start after a reboot

```@reboot /home/ubuntu/tinklaRelayHUD/tinklaRelayHud.sh```

provided that `/home/ubuntu/tinklaRelayHUD` is where you cloned the repo.

It is very important to run the script as root because it needs access to both the USB devices and the brightness controll.
