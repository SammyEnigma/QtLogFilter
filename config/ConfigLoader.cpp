#include "ConfigLoader.h"

ConfigLoader::ConfigLoader(QObject* parent)
    : QObject(parent)
{
    loadConfig();
}

ConfigLoader::~ConfigLoader() {
    saveConfig();
}

void ConfigLoader::setHost(const QHostAddress& address, int port) {
    this->address = address;
    this->port = port;
}

void ConfigLoader::addNewConfigData(const ConfigData& data) {
    configData << data;
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

    int size = setting.beginReadArray("filters");
    for (int i = 0; i < size; i++) {
        setting.setArrayIndex(i);
        ConfigData data;
        data.configName = setting.value("configName").toString();
        data.tagThreadName = setting.value("tagThreadName").toString();
        data.tag = setting.value("tag").toString();
        data.regEnable = setting.value("regEnable").toBool();
        data.logLevel = setting.value("logLevel").toInt();
        configData << data;
    }
    setting.endArray();
}

void ConfigLoader::saveConfig() {
    QSettings setting(".ini", QSettings::IniFormat);

    setting.beginGroup("service");
    setting.setValue("address", address.toString());
    setting.setValue("port", port);
    setting.endGroup();

    setting.beginWriteArray("filters");
    for (int i = 0; i < configData.size(); i++) {
        setting.setArrayIndex(i);
        ConfigData d = configData.at(i);
        setting.setValue("configName", d.configName);
        setting.setValue("tagThreadName", d.tagThreadName);
        setting.setValue("tag", d.tag);
        setting.setValue("regEnable", d.regEnable);
        setting.setValue("logLevel", d.logLevel);
    }
    setting.endArray();
}
