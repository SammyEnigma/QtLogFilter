# QtLogFilter
qt编写的一个日志筛选器

![image](https://raw.githubusercontent.com/daonvshu/QtLogFilter/master/preview.png)

1. 克隆项目，使用vs打开编译生成QtLogFilter

2. 拷贝LogTest下的2个文件到项目中：Log.h、Log.cpp

3. 使用Log.h连接到QtLogFilter
```c++
//等待连接到QtLogFilter并阻塞
Log::waitForConnect(QHostAddress::LocalHost, 56602);
//或者不连接到QtLogFilter，使用qdebug输出
Log::useQDebugOnly();
```

4. 日志打印格式
```
HH:mm:ss.zzz threadName-threadId logLevel/logTag: logMessage
```

5. 打印日志
```c++
//LogTest.cpp

#include "Log.h"
class TestThread : public QThread {
public:
    void run() override {
        //set current thread log name
        Log::setCurrentThreadName("logtest");

        Log::d("thrtag") << qrand(); //11:08:38.442 logtest-31068 D/thrtag: 29358

        //print thread exit
        Log::threadExit(); //11:08:40.443 logtest-31068 E/: thread exit!
    }
}

class LogTest : public QWidget {
public:
    LogTest() {
        ...
        connect(ui.pushButton, &QPushButton::clicked, [&] {
            //this will print two logs
            //11:08:41.984 main-37068 D/click: log by pushbutton -> 18467
            //11:08:41.984 main-37068 D/click: QFlags<Qt::WindowType>(Window|WindowTitleHint|WindowSystemMenuHint|WindowMinMaxButtonsHint|WindowCloseButtonHint|WindowFullscreenButtonHint)
            Log::d("click") << "log by pushbutton -> " + QString::number(qrand()) << this->windowFlags();

            //use macro
            Log_D("click") << "log by pushButton"; //11:08:41.984 main-37068 D/click: log by pushButton  (file: LogTest line:45)

            Log_d << "log by pushButton"; //11:08:41.984 main-37068 D/LogTest: log by pushButton  (line:46)
        });
        ...
    }
}

//qDebug to Logfilter
void myMessageOutput(QtMsgType type, const QMessageLogContext& context, const QString& msg) {
    QString log = msg;
    QString tag = "";
    if (context.file) {
        QFileInfo info(context.file);
        tag = info.baseName();
        log = QString("%1 (line:%3)").arg(msg).arg(context.line);
    }
    switch (type) {
    case QtDebugMsg:
    case QtInfoMsg:
        Log::d(tag) << log;
        break;
    case QtWarningMsg:
        Log::w(tag) << log;
        break;
    case QtCriticalMsg:
    case QtFatalMsg:
        Log::e(tag) << log;
        break;
    }
}

int main(int argc, char *argv[])
{
    ...
#ifdef QT_DEBUG
    qInstallMessageHandler(myMessageOutput);
    Log::waitForConnect(QHostAddress("192.168.1.x"), 56602);
#else
    Log::useQDebugOnly();
#endif // QT_DEBUG
    ...
}
```
