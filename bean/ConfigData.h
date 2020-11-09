#pragma once

#include <qstring.h>

struct ConfigData {
    QString configName;
    QString tagThreadName;
    QString tag;
    bool regEnable;
    int logLevel;
};
Q_DECLARE_METATYPE(ConfigData);