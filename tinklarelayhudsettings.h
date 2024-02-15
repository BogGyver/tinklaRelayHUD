#ifndef TINKLARELAYHUDSETTINGS_H
#define TINKLARELAYHUDSETTINGS_H

#include <QDialog>
#include <QSettings>

namespace Ui {
class tinklaRelayHudSettings;
}

class tinklaRelayHudSettings : public QDialog
{
    Q_OBJECT

public:
    explicit tinklaRelayHudSettings(QWidget *parent = nullptr);
    ~tinklaRelayHudSettings();
    QSettings *tinklaRelayAppSettings;
    void setExistingValues();
private slots:
    void cancelButton();
    void saveButton();
private:
    Ui::tinklaRelayHudSettings *ui;
};

#endif // TINKLARELAYHUDSETTINGS_H
