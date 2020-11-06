#include "LogReceiver.h"

#include <qnetworkproxy.h>
#include <qtcpsocket.h>
#include <qdebug.h>

LogReceiver::LogReceiver(QObject* parent) 
    : QObject(parent)
{
    tcpServer = new QTcpServer(this);
    tcpServer->setProxy(QNetworkProxy::NoProxy);

    connect(tcpServer, &QTcpServer::newConnection, this, &LogReceiver::acceptNewConnection);
}

void LogReceiver::addNewClient(QTcpSocket* client) {
    clients.insert(client, {});
    connect(client, qOverload<QAbstractSocket::SocketError>(&QAbstractSocket::error), [&, client] (QAbstractSocket::SocketError e) {
        QString errStr;
        QDebug(&errStr) << e;
        emit clientGotErr(clients.value(client).info, errStr);
    });

    connect(client, &QAbstractSocket::close, [&, client] {
        emit clientClosed(clients.take(client).info);
        client->deleteLater();
    });

    connect(client, &QAbstractSocket::readyRead, [&, client] {
        auto data = client->readAll();
        auto& clientData = clients[client];
        clientData.receiveBuffer.append(data);
        handleBuffer(clientData);
    });
}

void LogReceiver::handleBuffer(ClientData& clientData) {

}

void LogReceiver::acceptNewConnection() {
    while (tcpServer->hasPendingConnections()) {
        auto client = tcpServer->nextPendingConnection();
        addNewClient(client);
    }
}
