#include "QtLogFilter.h"

#include <qfile.h>
#include <qthread.h>
#include <qdatetime.h>
#include <qdebug.h>

#include "SmoothScrollbar.h"

#include "dialog/SettingDlg.h"
#include "dialog/FilterConfigDlg.h"

QtLogFilter::QtLogFilter(QWidget *parent)
    : ShadowWidget(parent)
{
	setWindowFlag(Qt::WindowStaysOnTopHint);
	setAttribute(Qt::WA_DeleteOnClose, false);
	setBackgroundColor(0xff252526);
	setBorderRadius(4);
	setDraggableTitleHeight(40);
    setupUi(ui);

    connect(ui.btn_close, &QPushButton::clicked, this, &QWidget::close);
    connect(ui.btn_setting, &QPushButton::clicked, [&] {
        auto dlg = new SettingDlg(this);
        dlg->show();
    });
    connect(ui.btn_add, &QPushButton::clicked, [&] {
        auto dlg = new FilterConfigDlg(this);
        dlg->show();
    });

    ui.textBrowser->setVerticalScrollBar(new SmoothScrollbar(this));

    logReceiver = new LogReceiver;
    connect(logReceiver, &LogReceiver::clientClosed, this, [&](ConnectData data) {

    });
    connect(logReceiver, &LogReceiver::clientConnneted, this, [&](ConnectData data) {

    });
    connect(logReceiver, &LogReceiver::clientGotLog, this, &QtLogFilter::showLog);

    logReceiver->validator = [&](const ClientData& clientData, const LogData& logData) {

        return true;
    };


    configLoader = new ConfigLoader(this);
    logReceiver->listen(configLoader->getAddress(), configLoader->getPort());
}

void QtLogFilter::showEvent(QShowEvent*) {
	move(QCursor::pos());
}

void QtLogFilter::showLog(LogData data) {
    switch (data.level) {
    case LEVEL_DEBUG:
        ui.textBrowser->setTextColor(Qt::white);
        break;
    case LEVEL_WARNING:
        ui.textBrowser->setTextColor(Qt::yellow);
        break;
    case LEVEL_ERROR:
        ui.textBrowser->setTextColor(Qt::red);
        break;
    default:
        break;
    }
    ui.textBrowser->append(data.toString());
}

