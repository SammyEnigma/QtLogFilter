#pragma once

#include "../ShadowWidget.h"

namespace Ui {
    class FilterConfig;
}

class FilterConfigDlg : public ShadowWidget {
public:
    FilterConfigDlg(QWidget* parent);
    ~FilterConfigDlg();

private:
    Ui::FilterConfig* ui;
};