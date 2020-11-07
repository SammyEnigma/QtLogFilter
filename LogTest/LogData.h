#pragma once

#include <qstring.h>
#include <qjsondocument.h>
#include <qjsonobject.h>
#include <qdatetime.h>

enum LogLevel {
    LEVEL_DEBUG = 0,
    LEVEL_WARNING,
    LEVEL_ERROR,
};

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

struct LogData {
    QString threadName;
    int64_t threadId;
    LogLevel level;
    qint64 time;
    QString log;

    QByteArray toTransData() {
        QJsonObject obj;
        obj.insert("a", threadName);
        obj.insert("b", threadId);
        obj.insert("c", (int)level);
        obj.insert("d", time);
        obj.insert("e", log);
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
            logStr += "D/ ";
            break;
        case LEVEL_WARNING:
            logStr += "W/ ";
            break;
        case LEVEL_ERROR:
            logStr += "E/ ";
            break;
        default:
            break;
        }

        logStr += log;

        return logStr;
    }
};
Q_DECLARE_METATYPE(LogData);
