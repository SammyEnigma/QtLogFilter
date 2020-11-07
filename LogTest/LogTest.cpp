#include "LogTest.h"

#include <qthread.h>
#include <qrandom.h>

#include "Log.h"

class TestThread : public QThread {
public:
    void run() override {
        testRunning = true;
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

bool TestThread::testRunning = true;

LogTest::LogTest() : QWidget(nullptr) {
    ui.setupUi(this);

    connect(ui.pushButton, &QPushButton::clicked, [&] {
        (new TestThread)->start();
    });

    connect(ui.pushButton_2, &QPushButton::clicked, [&] {
        Log::d("log by pushbutton -> " + QString::number(qrand()));
    });

    connect(ui.pushButton_3, &QPushButton::clicked, [&] {
        TestThread::testRunning = false;
    });
}

void LogTest::closeEvent(QCloseEvent*) {
    TestThread::testRunning = false;
}
