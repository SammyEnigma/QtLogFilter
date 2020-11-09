#pragma once

#include "QtNetwork/qtcpserver.h"

#include "../bean/LogData.h"
#include "../bean/ClientData.h"

#include <functional>
#include <qhostinfo.h>

class LogReceiver : public QObject {
    Q_OBJECT

public:
    LogReceiver();
    ~LogReceiver();

    void listen(const QHostAddress& address, int port);
    void reselectProcess(const ConnectData& data, bool death);
    void reloadProcessLog(const ConnectData& data, bool death);
    void clearLog(const ConnectData& data, bool death);

    std::function<bool(const ClientData&, const LogData&)> validator;
    std::function<void(const ClientData&, const QString&)> getNewThreadHandler;

signals:
    void clientClosed(ConnectData);
    void clientConnneted(ConnectData);
    void clientGotLog(LogData data);

private:
    QTcpServer* tcpServer;
    QHash<QTcpSocket*, ClientData> clients;
    QHash<ConnectData, ClientData> deathProcess;
    bool waitForClose;

private slots:
    void acceptNewConnection();

private:
    ClientData* getClient(const ConnectData& data, bool death);
    void addNewClient(QTcpSocket* client);
    void handleBuffer(ClientData& clientData);
    void tryProcessInfo(const QByteArray& data, ClientData& clientData);
};

