#ifndef TINKARELAYHUD_H
#define TINKARELAYHUD_H

#define TRHUD_W    800
#define TRHUD_H   480

#include <QMainWindow>
#include <QProcess>
#include <QTimer>
#include <QFile>
#include <QLabel>
#include <QSettings>
#include <array>
#include "tinklarelaydriver.h"

QT_BEGIN_NAMESPACE
namespace Ui { class TinklaRelayHUD; }
QT_END_NAMESPACE

class TinklaRelayHUD : public QMainWindow
{
    Q_OBJECT

public:
    TinklaRelayHUD(QWidget *parent = nullptr);
    ~TinklaRelayHUD();
    virtual void drawHud();
    virtual void startUpdateTimer(int interval);
    virtual void startSpinnerTimer(int interval);
    virtual void startUsbTimer(int interval);
    virtual void setBrightnessControllPath(QString path);
    bool flipV = false;
    bool flipH = false;
    int speedSignRegion = 0;
    QSettings *tinklaRelayAppSettings;
private slots:
    void screenUpdate();
    void drawSplash();
    void usbComm();
    void openSettings();
private:
    Ui::TinklaRelayHUD *ui;
    QPixmap accEnabled;
    QPixmap accAvailable;
    QPixmap apAvailable;
    QPixmap apEnabled;
    QFont mySpeedFont = QFont(":/img/gotham.ttf",88);
    QFont myAccFont = QFont(":/img/gothamNarrow.otf",28);
    QFont mySpeedLimitFont = QFont(":/img/gothamNarrow.otf",24);
    QFont mySplashScreenMessageFont = QFont(":/img/gothamNarrow.otf",24);
    QTimer *updateTimer_;
    QTimer *splashTimer_;
    QTimer *usbCommTimer_;

    TinklaRelayDriver myTr;

    int oldSpeedLimit = 0;
    int oldAccSpeed = 0;
    int oldSpeed = 0;

    void setSpeedLimit(uint8_t speed);
    void setAccLimit(uint8_t status, uint8_t speed);
    void setApStatus(bool AP_available, bool AP_on);
    void setGear(bool in_reverse, bool in_forward, bool in_neutral);
    void drawEnergy(int pwrUsed, int pwrAvailable);
    void setBlindSpot(bool leftBSM, bool rightBSM);
    void setLights(bool lightsOn, bool highBeamOn);
    void setTurnSignals(bool leftTs, bool rightTs);
    void setSpeed(int speed);
    void setTireAlert(bool tpmsAlert);
    void setBrakeHold(bool applied);
    void setSplash(bool isVisible);
    void flipLayout();
    void writeTextToLabel(QLabel *theLabel, QString theString, QFont theFont, QColor theColor);
    const int center_x = 240;
    const int center_y = 200;
    const int engRad = 180;
    const int qrtrVal = 120;
    const int posScale[4] = {40,80,160,320};
    const int negScale[2] = {-30,-60};
    const int posScaleLen = sizeof(posScale)/sizeof(posScale[0]);
    const int negScaleLen = sizeof(negScale)/sizeof(negScale[0]);
    //spinner stuff
    int numbSpinnerTracks = 30;
    int spinnerTrackPos = 0;
    std::array<QPixmap, 30> spinnerTrackImgs;
    QString spinnerText = "";
    void prepSpinnerTracks();
    bool brightnessEnabled = false;
    QString brightnessControllPath = "";
    void setBrightness(int brightness);
    int previousBrightness = 0;
};
#endif // TINKARELAYHUD_H
