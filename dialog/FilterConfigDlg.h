#pragma once

#include "../ShadowWidget.h"

#include "../bean/ConfigData.h" 

#include <functional>

namespace Ui {
    class FilterConfig;
}

class FilterConfigDlg : public ShadowWidget {
public:
    FilterConfigDlg(QWidget* parent);
    ~FilterConfigDlg();

    std::function<void(const ConfigData&)> newConfig;

private:
    Ui::FilterConfig* ui;
};