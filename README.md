# QtLogFilter
qt编写的一个日志筛选器

![image](https://raw.githubusercontent.com/daonvshu/QtLogFilter/master/preview.png)

1. 克隆项目，使用vs打开编译生成QtLogFilter

2. 拷贝LogTest下的3个文件到项目中：Log.h、LogData.h、Log.cpp

3. 使用Log.h连接到QtLogFilter
```c++
//等待连接到QtLogFilter并阻塞
Log::waitForConnect(QHostAddress::LocalHost, 56602);
//或者不连接到QtLogFilter，使用qdebug输出
Log::useQDebugOnly();
```
4. 打印日志
```c++
Log::d("tag", "log");
//追踪局部线程函数结束，使用Log::Local类
Log::Local local("thread name");
local.d("tag", "log");
```