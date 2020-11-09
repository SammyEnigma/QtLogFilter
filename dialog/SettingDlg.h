#pragma once

#include "../ShadowWidget.h"

#include <qhostinfo.h>
#include <functional>

namespace Ui {
    class Setting;
};

class SettingDlg : public ShadowWidget
{
public:
    SettingDlg(QWidget* parent);
    ~SettingDlg();
    
    void setConfig(const QHostAddress& address, int port);

    std::function<void(const QHostAddress&, int)> configChanged;

private:
    Ui::Setting* ui;
    QHostAddress oldAddr;
    int oldPort;

private:
    QStringList getLocalIps();
};

