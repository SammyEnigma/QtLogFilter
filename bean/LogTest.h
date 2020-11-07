#pragma once

#include <qrunnable.h>
#include <qthreadpool.h>

#include <qrandom.h>

#include "Log.h"

class LogTest : public QObject {
private:
    class TestThread : public QRunnable {
    public:
        void run() override {
            Log::waitForConnect(QHostAddress::LocalHost, 56602);

            Log::Local local("logtest");
            while (testRunning) {
                int level = qrand() % 3;
                switch (level) {
                case 0:
                    local.d(QString::number(qrand()));
                    break;
                case 1:
                    local.w(QString::number(qrand()));
                    break;
                case 2:
                    local.e(QString::number(qrand()));
                    break;
                }
                QThread::msleep(1000);
            }
        }

        static bool testRunning;
    };

public:
    LogTest(QObject* parent) : QObject(parent) {}
    ~LogTest() {
        TestThread::testRunning = false;
    }

    void startTest() {
        QThreadPool::globalInstance()->start(new TestThread());
    }

};

bool LogTest::TestThread::testRunning = true;