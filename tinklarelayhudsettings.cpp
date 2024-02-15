#include "tinklarelayhudsettings.h"
#include "ui_tinklarelayhudsettings.h"

tinklaRelayHudSettings::tinklaRelayHudSettings(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::tinklaRelayHudSettings)
{
    ui->setupUi(this);
    connect(ui->saveButton,SIGNAL(clicked()),this,SLOT(saveButton()));
    connect(ui->cancelButton,SIGNAL(clicked()),this,SLOT(cancelButton()));
}

tinklaRelayHudSettings::~tinklaRelayHudSettings()
{
    delete ui;
}

void tinklaRelayHudSettings::cancelButton() {
    close();
}

void tinklaRelayHudSettings::setExistingValues() {
    bool flipH = tinklaRelayAppSettings->value("FlipHorizontally", false).toBool();
    bool flipV = tinklaRelayAppSettings->value("FlipVertically", false).toBool();
    int speedSignRegion = tinklaRelayAppSettings->value("SpeedSignRegion",0).toInt();
    switch (speedSignRegion) {
       case 0:
            ui->radioUS->setChecked(true);
            break;
       case 1:
            ui->radioCA->setChecked(true);
            break;
        case 2:
            ui->radioROW->setChecked(true);
    }
    if ((!flipH) && (!flipV)) {
        ui->rotN->setChecked(true);
    }
    if ((!flipH) && (flipV)) {
        ui->rotV->setChecked(true);
    }
    if ((flipH) && (!flipV)) {
        ui->rotH->setChecked(true);
    }
    if ((flipH) && (flipV)) {
        ui->rotHV->setChecked(true);
    }
}

void tinklaRelayHudSettings::saveButton() {
    if (ui->rotN->isChecked()) {
        tinklaRelayAppSettings->setValue("FlipHorizontally",false);
        tinklaRelayAppSettings->setValue("FlipVertically",false);
    }
    if (ui->rotH->isChecked()) {
        tinklaRelayAppSettings->setValue("FlipHorizontally",true);
        tinklaRelayAppSettings->setValue("FlipVertically",false);
    }
    if (ui->rotHV->isChecked()) {
        tinklaRelayAppSettings->setValue("FlipHorizontally",true);
        tinklaRelayAppSettings->setValue("FlipVertically",true);
    }
    if (ui->rotV->isChecked()) {
        tinklaRelayAppSettings->setValue("FlipHorizontally",false);
        tinklaRelayAppSettings->setValue("FlipVertically",true);
    }
    if (ui->radioUS->isChecked()) {
        tinklaRelayAppSettings->setValue("SpeedSignRegion",0);
    }
    if (ui->radioCA->isChecked()) {
        tinklaRelayAppSettings->setValue("SpeedSignRegion",1);
    }
    if (ui->radioROW->isChecked()) {
        tinklaRelayAppSettings->setValue("SpeedSignRegion",2);
    }
    close();
    qApp->exit(1337);
}
