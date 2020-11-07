#include "ConfigLoader.h"

ConfigLoader::ConfigLoader(QObject* parent)
    : QObject(parent)
{
    loadConfig();
}

void ConfigLoader::loadConfig() {
    QSettings setting(".ini", QSettings::IniFormat);
    setting.beginGroup("service");
    address = setting.value("address", "").toString();
    if (address.isNull()) {
        address = QHostAddress::LocalHost;
    }
    port = setting.value("port", 56602).toInt();
    setting.endGroup();
}

void ConfigLoader::saveConfig() {
    
}
