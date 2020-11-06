#pragma once

#include <QtWidgets/QWidget>
#include "ui_QtLogFilter.h"

#include "ShadowWidget.h"

class QtLogFilter : public ShadowWidget
{
    Q_OBJECT

public:
    QtLogFilter(QWidget *parent = Q_NULLPTR);

protected:
    void showEvent(QShowEvent*) override;

private:
    Ui::QtLogFilterClass ui;

};
