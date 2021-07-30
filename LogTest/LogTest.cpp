#include "LogTest.h"

#include <qthread.h>
#include <qrandom.h>

#include "Log.h"

class TestThread : public QThread {
public:
    void run() override {
        testRunning = true;
        Log::setCurrentThreadName("logtest");
        while (testRunning) {
            int level = qrand() % 3;
            switch (level) {
            case 0:
                Log_D("debug") << qrand();
                break;
            case 1:
                Log_W("warning") << qrand();
                break;
            case 2:
                Log_E("error") << qrand();
                break;
            }
            QThread::msleep(1000);
        }
        Log::threadExit();
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
        Log_D("click") << "log by pushbutton ->" << qrand() << this->windowFlags();
        Log_D("click") << "log by pushButton";
        Log_d << "log by pushButton";
    });

    connect(ui.pushButton_3, &QPushButton::clicked, [&] {
        TestThread::testRunning = false;
    });
}

void LogTest::closeEvent(QCloseEvent*) {
    TestThread::testRunning = false;
}
