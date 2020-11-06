#pragma once

#include "LogData.h"

#include <qlist.h>

struct ClientData {
    ConnectData info;
    QList<LogData> data;
    QByteArray receiveBuffer;
};