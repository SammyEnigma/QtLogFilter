#include "Log.h"

#include "LogData.h"

#include <qcoreapplication.h>
#include <qthread.h>
#include <qdatetime.h>
#include <qthreadpool.h>
#include <qeventloop.h>
#include <qdebug.h>

Log* Log::instance = nullptr;
bool Log::LogReadWriteThread::readWriteRunning = true;

void Log::waitForConnect(const QHostAddress& address, int port) {
    if (instance == nullptr) {
        instance = new Log;
    }
    instance->connect(address, port);
}

void Log::d(const QString& log) {
    LogData data;
    data.level = LEVEL_DEBUG;
    addLog(data, log);
}

void Log::w(const QString& log) {
    LogData data;
    data.level = LEVEL_WARNING;
    addLog(data, log);
}

void Log::e(const QString& log) {
    LogData data;
    data.level = LEVEL_ERROR;
    addLog(data, log);
}

Log::Local::Local(const QString& threadName) {
    auto threadId = (int64_t)QThread::currentThreadId();
    instance->threadNames.insert(threadId, threadName);
}

Log::Local::~Local() {
    LogData data;
    data.level = LEVEL_ERROR;
    instance->addLog(data, "thread exit!");
}

void Log::Local::d(const QString& log) {
    instance->d(log);
}

void Log::Local::w(const QString& log) {
    instance->w(log);
}

void Log::Local::e(const QString& log) {
    instance->e(log);
}

Log::~Log() {
    LogReadWriteThread::readWriteRunning = false;
}

void Log::connect(const QHostAddress& address, int port) {
    QThreadPool::globalInstance()->start(new LogReadWriteThread(address, port));
    connectWaitLoop.exec();
}

void Log::addLog(LogData& data, const QString& log) {
    QMutexLocker lock(&instance->logQueueLock);

    data.threadId = (int64_t)QThread::currentThreadId();
    data.threadName = instance->threadNames.value(data.threadId);
    data.time = QDateTime::currentMSecsSinceEpoch();
    data.log = log;

    instance->logQueue.enqueue(data);
}

Log::LogReadWriteThread::LogReadWriteThread(const QHostAddress& address, int port)
    : address(address)
    , port(port)
{
}

void Log::LogReadWriteThread::run() {
    QTcpSocket* client;

    client = new QTcpSocket;

    QEventLoop loop;
    QEventLoop writeLoop;

    QObject::connect(client, &QTcpSocket::connected, [&] {
        clientIsConnected = true;
    });

    QObject::connect(client, qOverload<QAbstractSocket::SocketError>(&QTcpSocket::error), [&](QAbstractSocket::SocketError err) {
        qDebug() << err;
        loop.quit();
    });

    QObject::connect(client, &QTcpSocket::disconnected, [&] {
        clientIsConnected = false;
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

    QObject::connect(client, &QTcpSocket::bytesWritten, [&] {
        writeLoop.quit();
    });

    client->connectToHost(address, port);
    loop.exec();

    while (readWriteRunning) {
        if (!instance->logQueue.isEmpty()) {
            QMutexLocker lock(&instance->logQueueLock);
            auto log = instance->logQueue.dequeue();
            if (clientIsConnected) {
                client->write(log.toTransData() + ',');
                writeLoop.exec();
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
    obj.insert("processName", QCoreApplication::arguments().first());
    obj.insert("processId", QCoreApplication::applicationPid());
    QJsonDocument doc(obj);
    return doc.toJson();
}

