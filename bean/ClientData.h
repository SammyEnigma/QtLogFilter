#pragma once

#include "LogData.h"

#include <qlist.h>

struct ClientData {
    ConnectData info;
    QList<LogData> data;
    QStringList savedThreads;
    QByteArray receiveBuffer;
};