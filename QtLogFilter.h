#pragma once

#include <QtWidgets/QWidget>
#include "ui_QtLogFilter.h"

#include "ShadowWidget.h"

#include "bean/LogData.h" 

#include "receiver/LogReceiver.h"

#include "config/ConfigLoader.h"

#include <qstandarditemmodel.h>

class QtLogFilter : public ShadowWidget
{
    Q_OBJECT

public:
    QtLogFilter(QWidget *parent = Q_NULLPTR);

protected:
    void showEvent(QShowEvent*) override;

private:
    Ui::QtLogFilterClass ui;
    LogReceiver* logReceiver;
    ConfigLoader* configLoader;

    QStandardItemModel* processFilter;
    QStandardItemModel* threadFilter;
    QStandardItemModel* levelFilter;
    QStandardItemModel* configFilter;

private:
    void initThreadBox();
    void initLevelBox();
    void appendFilterConfig(const ConfigData& data);
    ConnectData getClientInfo(int row, bool& death);
    void reloadLog();

private slots:
    void showLog(LogData data);
    void newProcessArrived(ConnectData data);
    void processClosed(ConnectData data);

private:
    void newThreadArrived(const QString& name);
    void filterProcess(const ConnectData& processInfo);
    void filterThread(const LogData& data);
    void filterLevel(const LogData& data);
    void filterStr(const LogData& data);
    void filterConfig(const LogData& data);
};
