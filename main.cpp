#include "QtLogFilter.h"
#include <QtWidgets/QApplication>
#include <qdesktopwidget.h>

#ifdef QT_DEBUG
#include "qsswatcher/qsswatcher.h"
#endif // QT_DEBUG

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

#ifdef QT_DEBUG
    QssWatcher watcher;
#endif // QT_DEBUG

    QtLogFilter w(QApplication::desktop());
    w.show();

    return a.exec();
}
