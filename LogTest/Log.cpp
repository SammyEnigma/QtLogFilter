#include "Log.h"

#include "LogData.h"

#include <qcoreapplication.h>
#include <qthread.h>
#include <qdatetime.h>
#include <qthreadpool.h>
#include <qeventloop.h>
#include <qfileinfo.h>
#include <qdebug.h>

#include <qapplication.h>

Log* Log::instance = nullptr;
bool Log::LogReadWriteThread::readWriteRunning = true;

QMutex instanceLock;

void Log::waitForConnect(const QHostAddress& address, int port) {
    createInstance();
    instance->connect(address, port);
}

void Log::useQDebugOnly() {
    createInstance();
    instance->onlyQDebugPrint = true;
}

void Log::d(const QString& tag, const QString& log) {
    QMutexLocker lock1(&instanceLock);
    LogData data;
    data.level = LEVEL_DEBUG;
    addLog(data, tag, log);
}

void Log::w(const QString& tag, const QString& log) {
    QMutexLocker lock1(&instanceLock);
    LogData data;
    data.level = LEVEL_WARNING;
    addLog(data, tag, log);
}

void Log::e(const QString& tag, const QString& log) {
    QMutexLocker lock1(&instanceLock);
    LogData data;
    data.level = LEVEL_ERROR;
    addLog(data, tag, log);
}

void Log::release() {
    delete instance;
    instance = nullptr;
}

Log::Local::Local(const QString& threadName) {
    auto threadId = (int64_t)QThread::currentThreadId();
    instance->threadNames.insert(threadId, threadName);
}

Log::Local::~Local() {
    instanceLock.lock();
    if (instance != nullptr) {
        LogData data;
        data.level = LEVEL_ERROR;
        instance->addLog(data, "", "thread exit!");
    }
    instanceLock.unlock();
}

void Log::Local::d(const QString& tag, const QString& log) {
    instance->d(tag, log);
}

void Log::Local::w(const QString& tag, const QString& log) {
    instance->w(tag, log);
}

void Log::Local::e(const QString& tag, const QString& log) {
    instance->e(tag, log);
}

Log::~Log() {
    LogReadWriteThread::readWriteRunning = false;
}

void Log::createInstance() {
    instanceLock.lock();
    if (instance == nullptr) {
        instance = new Log;
        QObject::connect(qApp, &QCoreApplication::aboutToQuit, [&] {
            instanceLock.lock();
            delete instance;
            instance = nullptr;
            instanceLock.unlock();
        });
        instance->onlyQDebugPrint = false;
    }
    instanceLock.unlock();
}

void Log::connect(const QHostAddress& address, int port) {
    QThreadPool::globalInstance()->start(new LogReadWriteThread(address, port));
    connectWaitLoop.exec();
}

void Log::addLog(LogData& data, const QString& tag, const QString& log) {
    data.threadId = (int64_t)QThread::currentThreadId();
    data.threadName = instance->threadNames.value(data.threadId);
    data.time = QDateTime::currentMSecsSinceEpoch();
    data.log = log;
    data.tag = tag;

    if (instance == nullptr) {
        qDebug() << data.toString();
        return;
    }

    if (instance->onlyQDebugPrint) {
        qDebug() << data.toString();
        return;
    }

    QMutexLocker lock(&instance->logQueueLock);
    instance->logQueue.enqueue(data);
}

Log::LogReadWriteThread::LogReadWriteThread(const QHostAddress& address, int port)
    : address(address)
    , port(port) {
}

void Log::LogReadWriteThread::run() {
    QTcpSocket* client;

    client = new QTcpSocket;

    QEventLoop loop;

    QObject::connect(client, &QTcpSocket::connected, [&] {
        clientIsConnected = true;
    });

    QObject::connect(client, qOverload<QAbstractSocket::SocketError>(&QTcpSocket::error), [&](QAbstractSocket::SocketError err) {
        qDebug() << err;
        clientIsConnected = false;
        loop.quit();
    });

    QObject::connect(client, &QTcpSocket::readyRead, [&] {
        auto data = client->readAll();
        if (data.compare("who") == 0) {
            client->write(getProcessInfo());
        } else if (data.compare("ready") == 0) {
            instance->connectWaitLoop.exit();
            loop.quit();
        }
    });

    client->connectToHost(address, port);
    loop.exec();

    while (readWriteRunning) {
        QMutexLocker lock1(&instanceLock);
        if (instance == nullptr) {
            break;
        }
        QMutexLocker lock(&instance->logQueueLock);
        if (!instance->logQueue.isEmpty()) {
            auto log = instance->logQueue.dequeue();
            if (clientIsConnected) {
                client->write(log.toTransData() + ',');
                client->waitForBytesWritten();
            } else {
                qDebug() << log.toString();
            }
        }
    }

    client->close();
    delete client;
}

QByteArray Log::LogReadWriteThread::getProcessInfo() {
    QJsonObject obj;
    QFileInfo file(QCoreApplication::arguments().first());
    obj.insert("processName", file.baseName());
    obj.insert("processId", QCoreApplication::applicationPid());
    QJsonDocument doc(obj);
    return doc.toJson();
}

