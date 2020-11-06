#pragma once

#include "../ShadowWidget.h"

namespace Ui {
    class Setting;
};

class SettingDlg : public ShadowWidget
{
public:
    SettingDlg(QWidget* parent);
    ~SettingDlg();

private:
    Ui::Setting* ui;

private:
    QStringList getLocalIps();
};

