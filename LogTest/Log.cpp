#include "Log.h"

#include <qdatetime.h>
#include <qeventloop.h>
#include <qfileinfo.h>

#include <qjsondocument.h>
#include <qjsonobject.h>
#include <qdatetime.h>

#include <qdebug.h>

#include <qapplication.h>

struct ConnectData {
    QString processName;
    qint64 processId;
    bool clientReady;

    ConnectData() {
        processId = -1;
        clientReady = false;
    }
};
Q_DECLARE_METATYPE(ConnectData);

inline bool operator==(const ConnectData& e1, const ConnectData& e2) {
    return e1.processName == e2.processName
        && e1.processId == e2.processId;
}

inline uint qHash(const ConnectData& key, uint seed) {
    return qHash(key.processName, seed) ^ key.processId;
}

struct LogData {
    QString threadName;
    int64_t threadId;
    LogLevel level;
    qint64 time;
    QString log;
    QString tag;

    QByteArray toTransData() {
        QJsonObject obj;
        obj.insert("a", threadName);
        obj.insert("b", threadId);
        obj.insert("c", (int)level);
        obj.insert("d", time);
        obj.insert("e", log);
        obj.insert("f", tag);
        QJsonDocument doc(obj);
        return doc.toJson(QJsonDocument::Compact).toBase64();
    }

    void fromTransData(const QByteArray& data) {
        QJsonDocument doc = QJsonDocument::fromJson(QByteArray::fromBase64(data));
        if (!doc.isNull()) {
            auto obj = doc.object();
            threadName = obj.value("a").toString();
            threadId = obj.value("b").toVariant().value<int64_t>();
            level = (LogLevel)obj.value("c").toInt();
            time = obj.value("d").toVariant().toLongLong();
            log = obj.value("e").toString();
            tag = obj.value("f").toString();
        }
    }

    QString toString() {
        QString logStr;

        QDateTime t;
        t.setMSecsSinceEpoch(time);
        logStr += t.toString("HH:mm:ss.zzz ");

        if (!threadName.isEmpty()) {
            logStr += QString("%1-%2 ").arg(threadName).arg(threadId);
        }

        switch (level) {
        case LEVEL_DEBUG:
            logStr += "D/";
            break;
        case LEVEL_WARNING:
            logStr += "W/";
            break;
        case LEVEL_ERROR:
            logStr += "E/";
            break;
        default:
            break;
        }
        logStr += tag;
        logStr += ": ";
        logStr += log;

        return logStr;
    }
};
Q_DECLARE_METATYPE(LogData);

Log* Log::m_log = nullptr;
Log::Log() : onlyQDebugPrint(false) {
    connect(qApp, &QCoreApplication::aboutToQuit, [&] {
        quit();
        wait();
        delete m_log;
        m_log = nullptr;
    });
}

Log::~Log() {
}


void Log::createInstance() {
    if (m_log == nullptr) {
        m_log = new Log;
    }
}


void Log::waitForConnect(const QHostAddress& address, int port) {
    createInstance();
    m_log->address = address;
    m_log->port = port;
    m_log->start();
}

void Log::useQDebugOnly() {
    createInstance();
    m_log->onlyQDebugPrint = true;
}

void Log::run() {
    auto client = new QTcpSocket;
    QEventLoop connectLoop;

    connect(client, &QTcpSocket::readyRead, [&] {
        auto data = client->readAll();
        if (data.compare("who") == 0) {
            client->write(getProcessInfo());
        } else if (data.compare("ready") == 0) {
            connectLoop.quit();
        }
    });
    client->connectToHost(address, port);

    Q_ASSERT_X(client->waitForConnected(1000), "log", "cannot connect log filter!");

    connectLoop.exec();

    if (client->state() != QTcpSocket::ConnectedState) {
        onlyQDebugPrint = true;
        return;
    }

    connect(this, &Log::newLogArrived, client, [&] {
        QMutexLocker locker(&logQueueLock);
        QByteArray message;
        while (!logQueue.isEmpty()) {
            auto log = logQueue.dequeue();
            message.append(log.toTransData()).append(',');
        }
        client->write(message);
        client->waitForBytesWritten();
    }, Qt::QueuedConnection);

    exec();

    delete client;
}

void Log::setCurrentThreadName(const QString& name) {
    auto threadId = (int64_t)QThread::currentThreadId();
    QMutexLocker locker(&m_log->nameSetLock);
    m_log->threadNames.insert(threadId, name);
}

void Log::threadExit() {
    LogData data;
    data.level = LEVEL_ERROR;
    data.log = "thread exit!";
    addLog(data);
}

void Log::addLog(LogData& data) {
    data.threadId = (int64_t)QThread::currentThreadId();
    {
        QMutexLocker locker(&m_log->nameSetLock);
        data.threadName = m_log->threadNames.value(data.threadId);
        if (data.threadName.isEmpty()) {
            if (QThread::currentThread() == qApp->thread()) {
                data.threadName = "main";
            } else {
                data.threadName = "Thread-" + QString::number(data.threadId);
            }
            m_log->threadNames.insert(data.threadId, data.threadName);
        }
    }
    data.time = QDateTime::currentMSecsSinceEpoch();

    if (m_log->onlyQDebugPrint) {
        qDebug() << data.toString();
        return;
    }


    m_log->logQueueLock.lock();
    bool needPosSignal = m_log->logQueue.isEmpty();
    m_log->logQueue.enqueue(data);
    m_log->logQueueLock.unlock();
    if (needPosSignal) {
        emit m_log->newLogArrived(QPrivateSignal());
    }
}

QByteArray Log::getProcessInfo() {
    QJsonObject obj;
    QFileInfo file(QCoreApplication::arguments().first());
    obj.insert("processName", file.baseName());
    obj.insert("processId", QCoreApplication::applicationPid());
    QJsonDocument doc(obj);
    return doc.toJson();
}

void Log::Message::postLog(const QString& message) {
    QString extra;
    if (!fileName.isEmpty()) {
        QFileInfo fileInfo(fileName);
        if (tagIsFile) {
            tag = fileInfo.baseName();
            extra = QString(" (line:%1)").arg(line);
        } else {
            extra = QString(" (file: %1 line:%2)").arg(fileInfo.baseName()).arg(line);
        }
    }

    LogData logData;
    logData.tag = tag;
    logData.level = level;
    logData.log = message + extra;

    if (m_log == nullptr) {
        qDebug() << logData.toString();
        return;
    }

    addLog(logData);
}
