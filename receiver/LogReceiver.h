#pragma once

#include "QtNetwork/qtcpserver.h"

#include "../bean/LogData.h"
#include "../bean/ClientData.h"

class LogReceiver : public QObject {
    Q_OBJECT

public:
    LogReceiver(QObject* parent);

signals:
    void clientGotErr(ConnectData, QString);
    void clientClosed(ConnectData);

private:
    QTcpServer* tcpServer;
    QHash<QTcpSocket*, ClientData> clients;

private slots:
    void acceptNewConnection();

private:
    void addNewClient(QTcpSocket* client);
    void handleBuffer(ClientData& clientData);
};

