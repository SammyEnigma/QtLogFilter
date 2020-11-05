#include "QtLogFilter.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QtLogFilter w;
    w.show();
    return a.exec();
}
