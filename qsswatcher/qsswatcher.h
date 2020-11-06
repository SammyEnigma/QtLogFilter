#pragma once

#include <qobject.h>
#include <QtNetwork/qudpsocket.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qapplication.h>

class QssWatcher : public QObject {
public:
    QssWatcher(QObject* parent = nullptr) : QObject(parent) {
        udp = new QUdpSocket(this);
        connect(udp, &QUdpSocket::readyRead, this, &QssWatcher::dataReceived);
        udp->bind(QHostAddress::LocalHost, 12345);
    }

private slots:
    void dataReceived() {
        while (udp->hasPendingDatagrams()) {
            QByteArray data;
            data.resize(udp->pendingDatagramSize());
            udp->readDatagram(data.data(), data.size());
            solveQssPath(data);
        }
    }

private:
    QUdpSocket* udp;

    void solveQssPath(QString path) {
        QFile file(path);
        QFileInfo info(file);
        if (info.suffix() == "qss") {
            if (file.open(QIODevice::ReadOnly)) {
                QString qss = file.readAll();
                qApp->setStyleSheet(qss);
                file.close();
            }
        }
    }
};