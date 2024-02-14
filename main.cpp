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
    if (argc > 1) {
        w.setBrightnessControllPath(argv[1]);
    }
    return a.exec();
}
