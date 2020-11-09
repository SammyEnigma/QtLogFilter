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
        dlg->setConfig(configLoader->getAddress(), configLoader->getPort());
        dlg->configChanged = [&](const QHostAddress& addr, int port) {
            configLoader->setHost(addr, port);
            logReceiver->listen(configLoader->getAddress(), configLoader->getPort());
        };
    });
    connect(ui.btn_add, &QPushButton::clicked, [&] {
        auto dlg = new FilterConfigDlg(this);
        dlg->show();
        dlg->newConfig = [&](const ConfigData& data) {
            configLoader->addNewConfigData(data);
            appendFilterConfig(data);
        };
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
            filterStr(logData);
            filterConfig(logData);
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
    ui.box_filters->setModel((configFilter = new QStandardItemModel(this)));
    configFilter->appendRow(new QStandardItem("[no filter]"));
    ui.box_filters->setCurrentIndex(0);
    for (const auto& d : configLoader->getConfigs()) {
        appendFilterConfig(d);
    }
    
    connect(ui.box_process, qOverload<int>(&QComboBox::currentIndexChanged), [&] (int i) {
        ui.textBrowser->clear();
        initThreadBox();

        bool death;
        auto info = getClientInfo(i, death);
        logReceiver->reselectProcess(info, death);
    });

    connect(ui.box_thread, qOverload<int>(&QComboBox::currentIndexChanged), [&](int i) {
        reloadLog();
    });

    connect(ui.box_level, qOverload<int>(&QComboBox::currentIndexChanged), [&](int i) {
        reloadLog();
    });

    connect(ui.filter_str, &QLineEdit::textChanged, [&] (const QString&) {
        reloadLog();
    });

    connect(ui.check_reg, &QCheckBox::stateChanged, [&](int) {
        reloadLog();
    });

    connect(ui.btn_clear, &QPushButton::clicked, [&] {
        ui.textBrowser->clear();

        if (ui.box_process->currentIndex() == -1)
            return;

        bool death;
        auto info = getClientInfo(ui.box_process->currentIndex(), death);
        logReceiver->clearLog(info, death);
    });

    connect(ui.box_filters, qOverload<int>(&QComboBox::currentIndexChanged), [&](int i) {
        reloadLog();
    });
}

ConnectData QtLogFilter::getClientInfo(int row, bool& death) {
    auto index = processFilter->index(row, 0);
    auto info = index.data(Qt::UserRole + 1).value<ConnectData>();
    death = index.data(Qt::UserRole + 2).toBool();
    return info;
}

void QtLogFilter::reloadLog() {
    ui.textBrowser->clear();

    if (ui.box_process->currentIndex() == -1)
        return;

    bool death;
    auto info = getClientInfo(ui.box_process->currentIndex(), death);
    logReceiver->reloadProcessLog(info, death);
}

void QtLogFilter::showEvent(QShowEvent*) {
	move(QCursor::pos());
}

void QtLogFilter::initThreadBox() {
    ui.box_thread->blockSignals(true);
    threadFilter->clear();

    auto item = new QStandardItem(QStringLiteral("[ËùÓÐ]"));
    item->setData(-1, Qt::UserRole + 1);
    threadFilter->appendRow(item);

    item = new QStandardItem(QStringLiteral("[Î´ÉèÖÃ]"));
    item->setData(0, Qt::UserRole + 1);
    threadFilter->appendRow(item);

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

void QtLogFilter::appendFilterConfig(const ConfigData& data) {
    auto item = new QStandardItem(data.configName);
    item->setData(QVariant::fromValue(data), Qt::UserRole + 1);
    configFilter->appendRow(item);
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

void QtLogFilter::filterStr(const LogData& data) {
    auto tag = ui.filter_str->text();
    if (!tag.isEmpty()) {
        bool regEnable = ui.check_reg->isChecked();
        if (!regEnable) {
            if (QString("%1-%2").arg(data.threadName).arg(data.threadId).contains(tag)) {
                return;
            }
            if (data.tag.contains(tag)) {
                return;
            }
            if (data.log.contains(tag)) {
                return;
            }
        } else {
            QRegExp tag1(tag);
            if (QString("%1-%2").arg(data.threadName).arg(data.threadId).contains(tag1)) {
                return;
            }
            if (data.tag.contains(tag1)) {
                return;
            }
            if (data.log.contains(tag1)) {
                return;
            }
        }
        throw "filter str not match";
    }
}

void QtLogFilter::filterConfig(const LogData& data) {
    int currentIndex = ui.box_filters->currentIndex();
    if (currentIndex == -1) {
        return;
    }
    auto config = configFilter->index(currentIndex, 0).data(Qt::UserRole + 1);
    if (config.isNull()) {
        return;
    }
    auto configData = config.value<ConfigData>();
    if (!configData.tagThreadName.isEmpty()) {
        if (data.threadName != configData.tagThreadName) {
            throw "thread not match";
        }
    }
    if (!configData.tag.isEmpty()) {
        if (configData.regEnable) {
            QRegExp tag1(configData.tag);
            if (!data.tag.contains(tag1) && !data.log.contains(tag1)) {
                throw "tag not match";
            }
        } else {
            if (!data.tag.contains(configData.tag) && !data.log.contains(configData.tag)) {
                throw "tag not match";
            }
        }
    }
    if (data.level < configData.logLevel) {
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

