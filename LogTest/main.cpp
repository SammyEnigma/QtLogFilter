#include <QtWidgets/QApplication>

#include "Log.h"

#include "LogTest.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    Log::waitForConnect(QHostAddress::LocalHost, 56602);

    LogTest l;
    l.show();

    auto r = a.exec();
    return 0;
}
