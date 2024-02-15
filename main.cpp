#include "tinklarelayhud.h"

#include <QApplication>
#include <stdio.h>
#include <QCommandLineParser>



int main(int argc, char *argv[])
{
   int result = 0;
   do
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
        result =  a.exec();
     } while( result == 1337 );
     return result;
}
