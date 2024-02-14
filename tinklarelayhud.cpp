#include "tinklarelayhud.h"
#include "qpainter.h"
#include "cmath"
#include "ui_tinklarelayhud.h"

const float TIMER_INTERVAL = 100;
int pwr,pwr_jmp;
bool tinklaRelayConnected = false;
bool tinklaRelaySplashMode = false;
bool isStarting = false;

TinklaRelayHUD::TinklaRelayHUD(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::TinklaRelayHUD)
{
    ui->setupUi(this);
    QFont mySpeedFont = QFont(":/img/gotham.ttf",88);
    QFont myAccFont = QFont(":/img/gothamNarrow.otf",28);
    QFont mySpeedLimitFont = QFont(":/img/gothamNarrow.otf",24);
    accAvailable = QPixmap(":/img/accAvailable.png");
    accEnabled = QPixmap(":/img/accEnabled.png");
    ui->speedVal->setFont(mySpeedFont);
    ui->speedLimitValue->setFont(mySpeedLimitFont);
    ui->accSpeedValue->setFont(myAccFont);
    int region = 0; //0-US, 1-CA, 2-EU/ROW
    switch(region) {
        case 0:
            ui->speedLimitSign->setPixmap(QPixmap(":/img/speedLimitUS.png"));
            ui->speedLimitValue->setGeometry(ui->speedLimitValue->x(),ui->speedLimitValue->y()-2,
                                    ui->speedLimitValue->width(),ui->speedLimitValue->height());
            break;
        case 1:
            ui->speedLimitSign->setPixmap(QPixmap(":/img/speedLimitCA.png"));
            ui->speedLimitValue->setGeometry(ui->speedLimitValue->x(),ui->speedLimitValue->y()-2,
                                    ui->speedLimitValue->width(),ui->speedLimitValue->height());
            break;
        default:
            ui->speedLimitSign->setPixmap(QPixmap(":/img/speedLimitEU.png"));
            ui->speedLimitSign->setGeometry(ui->speedLimitSign->x(),ui->speedLimitSign->y()-15,
                                    ui->speedLimitSign->width(),ui->speedLimitSign->height());
            ui->speedLimitValue->setGeometry(ui->speedLimitValue->x(),ui->speedLimitValue->y()-20,
                                    ui->speedLimitValue->width(),ui->speedLimitValue->height());
            break;
    }
    isStarting = true;
    spinnerText = "Starting...";
    prepSpinnerTracks();
    updateTimer_ = new QTimer(this);
    splashTimer_ = new QTimer(this);
    usbCommTimer_ = new QTimer(this);
    connect(updateTimer_, SIGNAL(timeout()), this, SLOT(screenUpdate()));
    connect(splashTimer_, SIGNAL(timeout()), this, SLOT(drawSplash()));
    connect(usbCommTimer_, SIGNAL(timeout()), this, SLOT(usbComm()));
    pwr = 0;
    pwr_jmp = 3;
    flipV = false;
    flipH = false;
    flipLayout();
}

void TinklaRelayHUD::setBrightnessControllPath(QString path) {
    brightnessControllPath = path;
    brightnessEnabled = true;
}

void TinklaRelayHUD::flipLayout() {
    QList<QLabel *> list = ui->centralwidget->findChildren<QLabel *>();
    foreach(QLabel *l, list)
    {
        int x = l->geometry().x();
        int y = l->geometry().y();
        int w = l->geometry().width();
        int h = l->geometry().height();
        int nx = x;
        int ny = y;
        if (flipH) {
            nx = TRHUD_W - x - w;
            if (l->pixmap() != 0) {
                l->setPixmap(l->pixmap()->transformed(QTransform().scale(-1, 1)));
            }
        }
        if (flipV) {
           ny = TRHUD_H - y - h;
            if (l->pixmap() != 0) {
                l->setPixmap(l->pixmap()->transformed(QTransform().scale(1, -1)));
            }
        }
        l->setGeometry(nx,ny,w,h);
    }
}

void TinklaRelayHUD::setSpeedLimit(uint8_t speed) {
    if (speed == 0) {
        ui->speedLimitSign->setVisible(false);
        ui->speedLimitValue->setVisible(false);
    } else {
        ui->speedLimitSign->setVisible(true);
        ui->speedLimitValue->setVisible(true);
        ui->speedLimitValue->setText(QString::number(speed));
    }
}

void TinklaRelayHUD::setAccLimit(uint8_t status, uint8_t speed) {
    //0 unavailable, 1 available, 2 enabled
    if (status == 0) {
        ui->accSpeedSign->setVisible(false);
        ui->accSpeedValue->setVisible(false);
    } else {
        if (status == 1) {
            ui->accSpeedSign->setPixmap(accAvailable);
        } else {
            ui->accSpeedSign->setPixmap(accEnabled);
        }
        ui->accSpeedSign->setVisible(true);
        ui->accSpeedValue->setVisible(true);
        ui->accSpeedValue->setText(QString::number(speed));
    }
}

void TinklaRelayHUD::setGear(bool in_reverse, bool in_forward, bool in_neutral) {
    ui->maskGearD->setVisible(!in_forward);
    ui->maskGearN->setVisible(!in_neutral);
    ui->maskGearR->setVisible(!in_reverse);
    if (in_forward || in_reverse || in_neutral) {
        ui->maskGearP->setVisible(true);
     } else {
        ui->maskGearP->setVisible(false);
     }
}

void TinklaRelayHUD::setApStatus(bool AP_available,bool AP_on) {
    ui->apStatusAvailable->setVisible(AP_available);
    ui->apStatusEnabled->setVisible(AP_on);
}

void TinklaRelayHUD::drawEnergy(int pwrUsed) {
   int engScaled = 0;
   if (pwrUsed > 0) {
       engScaled = std::min(posScale[0],pwrUsed);
       for (int i=0;i<posScaleLen-1;i++) {
          if (pwrUsed > posScale[i]) {
            engScaled += (int)((std::min(posScale[i+1],pwrUsed) - posScale[i])/pow(2,i+1));
          }
       }
   }
   if (pwrUsed < 0) {
       engScaled = std::max(negScale[0],pwrUsed);
       for (int i=0;i<negScaleLen-1;i++) {
          if (pwrUsed < negScale[i]) {
            engScaled += (int)((std::max(negScale[i+1],pwrUsed) - negScale[i])/pow(2,i+1));
          }
       }
       //rescale like positive
       engScaled = (int)(engScaled * posScale[0] / std::abs(negScale[0]));
   }
   float centerAngleDeg = (90 * engScaled / qrtrVal);
   int angleSign = 1;
   if (centerAngleDeg != 0) {
       angleSign = (int)(centerAngleDeg/std::abs(centerAngleDeg));
   }
   if (std::abs(centerAngleDeg) <= 2) {
       angleSign = 0;
   }
   centerAngleDeg = centerAngleDeg - 2 * angleSign;
   //now draw
   QPixmap pixmap(ui->energyBar->width(),ui->energyBar->height());
   pixmap.fill(QColor("transparent"));
   QPainter painter(&pixmap);

   QRectF rectangle(center_x- engRad, center_y - engRad, 2*engRad, 2*engRad);
   int startAngle = 0;
   if (flipH) {
       startAngle = 180 * 16;
   }
   int spanAngle = (centerAngleDeg) * 16;
   if ((flipH != flipV) && (flipH || flipV)) {
       spanAngle = - spanAngle;
   }

   QPen pen;
   pen.setColor("orange");
   pen.setWidth(15);
   pen.setJoinStyle(Qt::RoundJoin);
   if (pwrUsed < 0) {
       pen.setBrush(Qt::green);
   }
   painter.setPen(pen);
   painter.drawArc(rectangle, startAngle, spanAngle);
   //draw marker
   int x = (int)((engRad -10) * cos((centerAngleDeg + 1 * angleSign)*3.14159/180));
   int y = (int)((engRad -10) * sin((centerAngleDeg + 1 * angleSign)*3.14159/180));
   int lineAngle = centerAngleDeg;
   if (flipH) {
       x = -x;
       lineAngle = 90+lineAngle;
   }
   if (flipV) {
       y = -y;
       lineAngle = -lineAngle;
   }
   pen.setColor("white");
   pen.setWidth(4);
   painter.setPen(pen);
   QLineF marker;
   marker.setP1(QPointF(center_x+x,center_y-y));
   marker.setAngle(lineAngle);
   marker.setLength(25);
   painter.drawLine(marker);
   ui->energyBar->setPixmap(pixmap);
}

void TinklaRelayHUD::setBlindSpot(bool leftBSM, bool rightBSM) {
    ui->hideLeftBsm->setVisible(!leftBSM);
    ui->hideRightBsm->setVisible(!rightBSM);
}

void TinklaRelayHUD::setLights(bool lightsOn, bool highBeamOn) {
    ui->hideLoBeam->setVisible(!lightsOn);
    ui->hideHiBeam->setVisible(!highBeamOn);
}

void TinklaRelayHUD::setTurnSignals(bool leftTs, bool rightTs) {
    ui->hideLeftTs->setVisible(!leftTs);
    ui->hideRightTs->setVisible(!rightTs);
}

void TinklaRelayHUD::setSpeed(int speed) {
    ui->speedVal->setText(QString::number(speed));
}

void TinklaRelayHUD::setTireAlert(bool tpmsAlert) {
    ui->hideLoTirePres->setVisible(!tpmsAlert);
}

void TinklaRelayHUD::setBrakeHold(bool applied) {
    ui->hideBrakeHold->setVisible(!applied);
}

void TinklaRelayHUD::drawHud()
{
   setSpeed(myTr.rel_speed);
   setSpeedLimit(myTr.rel_speed_limit);
   setAccLimit(myTr.rel_acc_status,myTr.rel_acc_speed);
   setApStatus(myTr.rel_AP_available,myTr.rel_AP_on);
   setGear(myTr.rel_gear_in_reverse, myTr.rel_gear_in_forward, myTr.rel_gear_in_neutral);
   setBlindSpot(myTr.rel_left_side_bsm,myTr.rel_right_side_bsm);
   setLights(myTr.rel_light_on,myTr.rel_highbeams_on);
   setTurnSignals(myTr.rel_left_turn_signal,myTr.rel_right_turn_signal);
   setTireAlert(myTr.rel_tpms_alert_on);
   setBrakeHold(myTr.rel_brake_hold_on);
   drawEnergy(myTr.rel_power_lvl);
   ui->zzzCarOff->setVisible((!myTr.rel_car_on) && (!tinklaRelaySplashMode) && (!isStarting));
   setBrightness((int)(myTr.rel_brightness * 2.55));
}

void TinklaRelayHUD::prepSpinnerTracks() {
  QPixmap track_img = QPixmap(":/img/spinnerTrack.png");

  QTransform transform(1, 0, 0, 1, 180, 180);
  QPixmap pm(QSize(360, 360));
  QPainter p(&pm);
  p.setRenderHint(QPainter::SmoothPixmapTransform);
  for (int i = 0; i < numbSpinnerTracks; ++i) {
    QTransform tr;
    tr.translate(180, 180);
    tr.rotate(i * 360 / numbSpinnerTracks);
    tr.translate(-180, -180);
    spinnerTrackImgs[i] = track_img.transformed(tr);
  }
}

void TinklaRelayHUD::setSplash(bool isVisible) {
    tinklaRelaySplashMode = isVisible;
    isStarting = false;
    ui->zSpinnerBkg->setVisible(tinklaRelaySplashMode);
    ui->zSpinnerTrack->setVisible(tinklaRelaySplashMode);
    ui->zSpinnerText->setVisible(tinklaRelaySplashMode);
    ui->zzzCarOff->setVisible((!tinklaRelaySplashMode) && (!isStarting));
    setBrightness(255);
}

void TinklaRelayHUD::drawSplash() {
    ui->zSpinnerTrack->setPixmap(spinnerTrackImgs[spinnerTrackPos]);
    ui->zSpinnerText->setText(spinnerText);
    spinnerTrackPos = (spinnerTrackPos + 1) % numbSpinnerTracks;
    if (spinnerTrackPos == 0) {
        spinnerText = "Searching for Tinkla Relay...";
    }
}

void TinklaRelayHUD::startUsbTimer(int interval) {
    usbCommTimer_->start(interval);
}

void TinklaRelayHUD::startUpdateTimer(int interval) {
    setSplash(false);
    splashTimer_->stop();
    updateTimer_->start(interval);
}

void TinklaRelayHUD::startSpinnerTimer(int interval) {
    setSplash(true);
    updateTimer_->stop();
    splashTimer_->start(interval);
}

void TinklaRelayHUD::setBrightness(int brightness) {
    if (!brightnessEnabled) return;
    if (brightness == previousBrightness) return;
    if (!myTr.rel_car_on) brightness = 0;
    QFile brightnessFile(brightnessControllPath);
    brightnessFile.open(QIODevice::WriteOnly | QIODevice::Truncate);
    brightnessFile.write(QString::number(brightness).toUtf8());
    brightnessFile.close();
    previousBrightness = brightness;
}

void TinklaRelayHUD::screenUpdate()
{
     drawHud();
}

void TinklaRelayHUD::usbComm() {
    if(myTr.disconnected()) {
        if (tinklaRelayConnected) {
            //we were connected before
            tinklaRelayConnected = false;
            startSpinnerTimer(50);
            myTr.close();
            //printf("Got disconnected!\n");
        }
        int errcnt = 0;
        QString errstr;
        QStringList trDevs = TinklaRelayDriver::listDevices(errcnt, errstr);
        if (errcnt > 0) {
            //error, return
            //printf("Error on list devices!\n");
            return;
        } else {
            if (trDevs.length() > 0) {
                //found device
                int err = myTr.open(trDevs[0]);
                if (err == TinklaRelayDriver::SUCCESS) {  // Device was successfully opened
                    //printf("Got connected!\n");
                    return;
                } else if (err == TinklaRelayDriver::ERROR_INIT) {  // Failed to initialize libusb
                    //printf("Failed to initialize libusb!\n");
                    return;
                } else {
                    if (err == TinklaRelayDriver::ERROR_NOT_FOUND) {  // Failed to find device
                        //printf("Failed to find device!\n");
                        return;
                    } else if (err == TinklaRelayDriver::ERROR_BUSY) {  // Failed to claim interface
                        //printf("Failed to claim interface!\n");
                        return;
                    }
                }
            }
        }
     } else {
        if (!tinklaRelayConnected) {
            tinklaRelayConnected = true;
            startUpdateTimer(100);
        }
        if (myTr.getData()) {
            //printf("Got data and processing it!\n");
            myTr.processDataMessage();
        } else {
            //printf("Failed to get data!\n");
        }

    }
}

TinklaRelayHUD::~TinklaRelayHUD()
{
    delete ui;
}

