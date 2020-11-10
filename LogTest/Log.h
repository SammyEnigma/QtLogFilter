#pragma once

#include <qhostinfo.h>
#include <qeventloop.h>
#include <qtcpsocket.h>
#include <qrunnable.h>
#include <qfileinfo.h>

#include <qqueue.h>
#include <qmutex.h>

#include "LogData.h"

class Log {
public:
    static void waitForConnect(const QHostAddress& address, int port);
    static void useQDebugOnly();

    static void d(const QString& tag, const QString& log);
    static void w(const QString& tag, const QString& log);
    static void e(const QString& tag, const QString& log);

    static void release();

    class Local {
    public:
        Local(const QString& threadName);
        ~Local();

        void d(const QString& tag, const QString& log);
        void w(const QString& tag, const QString& log);
        void e(const QString& tag, const QString& log);

    };

    class Message {
    public:
        Message(const QString& tag, LogLevel level, const char* fileName, int line)
            : tag(tag) , level(level), fileName(fileName), line(line) {
            tagIsFile = false;
        }

        Message(LogLevel level, const char* fileName, int line)
            : level(level), fileName(fileName), line(line) {
            tagIsFile = true;
        }

        void operator<<(const QString& msg) {
            QFileInfo fileInfo(fileName);
            QString extra;
            if (tagIsFile) {
                tag = fileInfo.baseName();
                extra = QString(" (line:%1)").arg(line);
            } else {
                extra = QString(" (file: %1 line:%2)").arg(fileInfo.baseName()).arg(line);
            }
            switch (level) {
            case LEVEL_DEBUG:
                Log::d(tag, msg + extra);
                break;
            case LEVEL_WARNING:
                Log::w(tag, msg + extra);
                break;
            case LEVEL_ERROR:
                Log::e(tag, msg + extra);
                break;
            default:
                break;
            }
        }

    private:
        QString tag, fileName;
        int line;
        LogLevel level;
        bool tagIsFile;
    };

private:
    Log() {};
    ~Log();
    Log(const Log&) = delete;
    void operator=(const Log&) = delete;

    static void createInstance();

private:
    static Log* instance;

    QEventLoop connectWaitLoop;
    QHash<int64_t, QString> threadNames;
    QQueue<LogData> logQueue;
    QMutex logQueueLock;
    bool onlyQDebugPrint;

private:
    void connect(const QHostAddress& address, int port);
    static void addLog(LogData& data, const QString& tag, const QString& log);

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

#define Log_D(tag)     Log::Message(tag, LEVEL_DEBUG, __FILE__, __LINE__)
#define Log_d             Log::Message(LEVEL_DEBUG, __FILE__, __LINE__)

#define Log_W(tag)    Log::Message(tag, LEVEL_WARNING, __FILE__, __LINE__)
#define Log_w             Log::Message(LEVEL_WARNING, __FILE__, __LINE__)

#define Log_E(tag)     Log::Message(tag, LEVEL_ERROR, __FILE__, __LINE__)
#define Log_e             Log::Message(LEVEL_ERROR, __FILE__, __LINE__)