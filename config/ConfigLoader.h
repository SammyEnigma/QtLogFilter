#pragma once

#include <qobject.h>
#include <qsettings.h>
#include <qhostinfo.h>

class ConfigLoader : public QObject {
public:
    ConfigLoader(QObject* parent);

    const QHostAddress& getAddress() {
        return address;
    }

    const int& getPort() {
        return port;
    }

private:
    QHostAddress address;
    int port;

private:
    void loadConfig();
    void saveConfig();
};

