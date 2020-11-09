#pragma once

#include <qobject.h>
#include <qsettings.h>
#include <qhostinfo.h>

#include "../bean/ConfigData.h"

class ConfigLoader : public QObject {
public:
    ConfigLoader(QObject* parent);
    ~ConfigLoader();

    const QHostAddress& getAddress() {
        return address;
    }

    const int& getPort() {
        return port;
    }

    const QList<ConfigData>& getConfigs() {
        return configData;
    }

    void setHost(const QHostAddress& address, int port);
    void addNewConfigData(const ConfigData& data);

private:
    QHostAddress address;
    int port;
    QList<ConfigData> configData;

private:
    void loadConfig();
    void saveConfig();
};

