#include "device_thread.h"
#include <QThread>

device_thread::device_thread(int id, QObject *parent) : QObject(parent), m_id(id)
{
}

device_thread::~device_thread(){};

void device_thread::doWork()
{
    // 在子线程中执行的代码
    qDebug() << "Working in thread" << m_id << "...";

    // 模拟一些耗时操作
    for (int i = 0; i < 1000000000; ++i) {
        // ...
    }

    emit workFinished(); // 发送工作完成信号
}

