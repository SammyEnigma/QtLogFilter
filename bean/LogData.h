#pragma once

#include <qstring.h>
#include <qjsondocument.h>
#include <qjsonobject.h>

enum LogLevel {
    LEVEL_DEBUG,
    LEVEL_WARNING,
    LEVEL_ERROR,
};

struct ConnectData {
    QString processName;
    int processId;
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
        return doc.toJson(QJsonDocument::Compact);
    }

    void fromTransData(const QByteArray& data) {
        QJsonDocument doc = QJsonDocument::fromJson(data);
        if (!doc.isNull()) {
            auto obj = doc.object();
            threadName = obj.value("a").toString();
            threadId = obj.value("b").toVariant().value<int64_t>();
            level = (LogLevel)obj.value("c").toInt();
            time = obj.value("d").toVariant().toLongLong();
            log = obj.value("e").toString();
        }
    }
};
Q_DECLARE_METATYPE(LogData);
