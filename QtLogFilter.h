#pragma once

#include <QtWidgets/QWidget>
#include "ui_QtLogFilter.h"

class QtLogFilter : public QWidget
{
    Q_OBJECT

public:
    QtLogFilter(QWidget *parent = Q_NULLPTR);

private:
    Ui::QtLogFilterClass ui;
};
