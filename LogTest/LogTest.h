#pragma once

#include <qwidget.h>

#include "ui_LogTest.h"

class LogTest : public QWidget {
public:
    LogTest();

private:
    Ui::Form ui;

protected:
    void closeEvent(QCloseEvent*) override;
};

