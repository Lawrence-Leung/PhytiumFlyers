#ifndef DEVICE_THREAD_H
#define DEVICE_THREAD_H

#include <QThread>
#include <QObject>
#include <QDebug>

class device_thread : public QObject
{
    Q_OBJECT
public:
    explicit device_thread(int id,QObject *parent = nullptr);
    device_thread(int id) : m_id(id) {};
    ~device_thread();

signals:
    void* dataUpdate();
    void threadFinished();
    void workFinished();

private:
    void  accepted();
    void doWork(); // 在子线程中执行的槽函数
    int m_id; // 线程标识号


};

#endif // DEVICE_THREAD_H
