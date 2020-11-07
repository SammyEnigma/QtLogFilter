#pragma once

#include <qhostinfo.h>
#include <qeventloop.h>
#include <qtcpsocket.h>
#include <qrunnable.h>

#include <qqueue.h>
#include <qmutex.h>

struct LogData;

class Log {
public:
    static void waitForConnect(const QHostAddress& address, int port);

    static void d(const QString& log);
    static void w(const QString& log);
    static void e(const QString& log);

    class Local {
    public:
        Local(const QString& threadName);
        ~Local();

        void d(const QString& log);
        void w(const QString& log);
        void e(const QString& log);

    };

private:
    Log() {};
    ~Log();
    Log(const Log&) = delete;
    void operator=(const Log&) = delete;

private:
    static Log* instance;

    QEventLoop connectWaitLoop;
    QHash<int64_t, QString> threadNames;
    QQueue<LogData> logQueue;
    QMutex logQueueLock;

private:
    void connect(const QHostAddress& address, int port);
    static void addLog(LogData& data, const QString& log);

private:
    class LogReadWriteThread : public QRunnable {
    public:
        LogReadWriteThread(const QHostAddress& address, int port);

        static bool readWriteRunning;

    protected:
        void run() override;

    private:
        QHostAddress address;
        int port;
        bool clientIsConnected;

    private:
        QByteArray getProcessInfo();
    };
};