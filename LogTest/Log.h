#pragma once

#include <qhostinfo.h>
#include <qtcpsocket.h>

#include <qqueue.h>
#include <qmutex.h>

#include <qthread.h>

enum LogLevel {
    LEVEL_DEBUG = 0,
    LEVEL_WARNING,
    LEVEL_ERROR,
};

class LogData;
class Log : public QThread {
    Q_OBJECT

public:
    static void waitForConnect(const QHostAddress& address, int port);
    static void useQDebugOnly();

    static void setCurrentThreadName(const QString& name);

    static void threadExit();

    class Message {
    public:
        Message(const QString& tag, LogLevel level, const char* fileName, int line)
            : tag(tag), level(level), fileName(fileName), line(line) {
            tagIsFile = false;
        }

        Message(LogLevel level, const char* fileName, int line)
            : level(level), fileName(fileName), line(line) {
            tagIsFile = true;
        }

        Message(const QString& tag, LogLevel level)
            : tag(tag), level(level) {
        }

        template<typename T>
        Message& operator<<(const T& msg) {
            postLog(messageToStr(msg));
            return *this;
        }

    private:
        QString tag, fileName;
        int line;
        LogLevel level;
        bool tagIsFile;

    private:
        void postLog(const QString& message);

        template<typename T>
        QString messageToStr(const T& msg) {
            QString messageStr;
            QDebug(&messageStr) << msg;
            return messageStr;
        }

        template<>
        QString messageToStr(const QString& message) {
            return message;
        }
    };

    static Message d(const QString& tag) {
        return Message(tag, LEVEL_DEBUG);
    }

    static Message w(const QString& tag) {
        return Message(tag, LEVEL_WARNING);
    }

    static Message e(const QString& tag) {
        return Message(tag, LEVEL_ERROR);
    }

signals:
    void newLogArrived(QPrivateSignal);

private:
    Log();
    ~Log();

    Q_DISABLE_COPY(Log);

    static void createInstance();

private:
    QHash<int64_t, QString> threadNames;
    QQueue<LogData> logQueue;
    QMutex logQueueLock;
    bool onlyQDebugPrint;

    QHostAddress address;
    int port;
    QMutex nameSetLock;

    static Log* m_log;

protected:
    void run() override;

private:
    static void addLog(LogData& data);

    QByteArray getProcessInfo();
};

#define Log_D(tag)     Log::Message(tag, LEVEL_DEBUG, __FILE__, __LINE__)
#define Log_d             Log::Message(LEVEL_DEBUG, __FILE__, __LINE__)

#define Log_W(tag)    Log::Message(tag, LEVEL_WARNING, __FILE__, __LINE__)
#define Log_w             Log::Message(LEVEL_WARNING, __FILE__, __LINE__)

#define Log_E(tag)     Log::Message(tag, LEVEL_ERROR, __FILE__, __LINE__)
#define Log_e             Log::Message(LEVEL_ERROR, __FILE__, __LINE__)