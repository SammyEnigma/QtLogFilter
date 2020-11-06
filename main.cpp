#include "QtLogFilter.h"
#include <QtWidgets/QApplication>
#include <qdesktopwidget.h>

#include "qsswatcher/qsswatcher.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QssWatcher watcher;

    QtLogFilter w(QApplication::desktop());
    w.show();

    return a.exec();
}
