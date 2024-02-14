#include "tinklarelayhud.h"

#include <QApplication>
#include <stdio.h>
#include <QCommandLineParser>



int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    TinklaRelayHUD w;
    w.setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    w.show();
    w.drawHud();
    w.startSpinnerTimer(50);
    w.startUsbTimer(200);
    w.tinklaRelayAppSettings = new QSettings("./tinklaRelaySettings.ini",QSettings::NativeFormat);
    if (argc > 1) {
        w.setBrightnessControllPath(argv[1]);
    }
    w.flipH = w.tinklaRelayAppSettings->value("FlipHorizontally", false).toBool();
    w.flipV = w.tinklaRelayAppSettings->value("FlipVertically", false).toBool();
    w.speedSignRegion = w.tinklaRelayAppSettings->value("SpeedSignRegion",0).toInt();
    return a.exec();
}
