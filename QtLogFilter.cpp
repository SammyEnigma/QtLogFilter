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
    connect(logReceiver, &LogReceiver::clientClosed, this, &QtLogFilter::processClosed);
    connect(logReceiver, &LogReceiver::clientConnneted, this, &QtLogFilter::newProcessArrived);
    connect(logReceiver, &LogReceiver::clientGotLog, this, &QtLogFilter::showLog);

    logReceiver->validator = [&](const ClientData& clientData, const LogData& logData) {
        try {
            filterProcess(clientData.info);
            filterThread(logData);
            filterLevel(logData);
        }
        catch (...) {
            return false;
        }
        return true;
    };

    logReceiver->getNewThreadHandler = [&](const ClientData& clientData, const QString& threadName) {
        try {
            filterProcess(clientData.info);
            newThreadArrived(threadName);
        } catch (...) {}
    };

    configLoader = new ConfigLoader(this);
    logReceiver->listen(configLoader->getAddress(), configLoader->getPort());

    ui.box_process->setModel((processFilter = new QStandardItemModel(this)));
    ui.box_thread->setModel((threadFilter = new QStandardItemModel(this)));
    initThreadBox();
    ui.box_level->setModel((levelFilter = new QStandardItemModel(this)));
    initLevelBox();

    
    connect(ui.box_process, qOverload<int>(&QComboBox::currentIndexChanged), [&] (int i) {
        ui.textBrowser->clear();
        initThreadBox();

        auto index = processFilter->index(i, 0);
        auto info = index.data(Qt::UserRole + 1).value<ConnectData>();
        bool death = index.data(Qt::UserRole + 2).toBool();
        logReceiver->reselectProcess(info, death);
    });

    connect(ui.box_thread, qOverload<int>(&QComboBox::currentIndexChanged), [&](int i) {
        reloadLog();
    });

    connect(ui.box_level, qOverload<int>(&QComboBox::currentIndexChanged), [&](int i) {
        reloadLog();
    });
}

void QtLogFilter::reloadLog() {
    ui.textBrowser->clear();

    if (ui.box_process->currentIndex() == -1)
        return;

    auto index = processFilter->index(ui.box_process->currentIndex(), 0);
    auto info = index.data(Qt::UserRole + 1).value<ConnectData>();
    bool death = index.data(Qt::UserRole + 2).toBool();
    logReceiver->reloadProcessLog(info, death);
}

void QtLogFilter::showEvent(QShowEvent*) {
	move(QCursor::pos());
}

void QtLogFilter::initThreadBox() {
    threadFilter->clear();

    auto item = new QStandardItem(QStringLiteral("[ËùÓÐ]"));
    item->setData(-1, Qt::UserRole + 1);
    threadFilter->appendRow(item);

    item = new QStandardItem(QStringLiteral("[Î´ÉèÖÃ]"));
    item->setData(0, Qt::UserRole + 1);
    threadFilter->appendRow(item);

    ui.box_thread->blockSignals(true);
    ui.box_thread->setCurrentIndex(0);
    ui.box_thread->blockSignals(false);
}

void QtLogFilter::initLevelBox() {
    auto item = new QStandardItem("debug");
    item->setData(0, Qt::UserRole + 1);
    levelFilter->appendRow(item);

    item = new QStandardItem("warning");
    item->setData(1, Qt::UserRole + 1);
    levelFilter->appendRow(item);

    item = new QStandardItem("error");
    item->setData(2, Qt::UserRole + 1);
    levelFilter->appendRow(item);
}

void QtLogFilter::newProcessArrived(ConnectData data) {
    auto item = new QStandardItem(QString("%1 - %2").arg(data.processName).arg(data.processId));
    item->setData(QVariant::fromValue(data), Qt::UserRole + 1);
    item->setData(false, Qt::UserRole + 2);
    processFilter->appendRow(item);
    if (ui.box_process->currentIndex() == -1) {
        ui.box_process->setCurrentIndex(0);
    }
}

void QtLogFilter::processClosed(ConnectData data) {
    for (int i = 0; i < processFilter->rowCount(); i++) {
        auto index = processFilter->index(i, 0);
        auto info = index.data(Qt::UserRole + 1).value<ConnectData>();
        if (info == data) {
            processFilter->setData(index, true, Qt::UserRole + 2);
            break;
        }
    }
}

void QtLogFilter::newThreadArrived(const QString& name) {
    auto item = new QStandardItem(name);
    item->setData(1, Qt::UserRole + 1);
    threadFilter->appendRow(item);
}

void QtLogFilter::filterProcess(const ConnectData& processInfo) {
    if (ui.box_process->currentIndex() == -1)
        throw "not select process";
    auto info = processFilter->index(ui.box_process->currentIndex(), 0).data(Qt::UserRole + 1).value<ConnectData>();
    if (info.processName != processInfo.processName || info.processId != processInfo.processId)
        throw "process not match";
}

void QtLogFilter::filterThread(const LogData& data) {
    if (ui.box_thread->currentIndex() == -1) {
        return;
    }
    auto index = threadFilter->index(ui.box_thread->currentIndex(), 0);
    int type = index.data(Qt::UserRole + 1).toInt();
    if (type == 0) {
        if (!data.threadName.isEmpty()) {
            throw "thread not match";
        }
    } else if (type == 1) {
        auto tagName = index.data(Qt::DisplayRole).toString();
        if (tagName != data.threadName) {
            throw "thread not match";
        }
    }
}

void QtLogFilter::filterLevel(const LogData& data) {
    int tagLevel = levelFilter->index(ui.box_level->currentIndex(), 0).data(Qt::UserRole + 1).toInt();
    if (data.level < tagLevel) {
        throw "level not match";
    }
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

