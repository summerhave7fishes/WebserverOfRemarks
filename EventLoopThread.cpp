#include "EventLoopThread.h"
#include <functional>

EventLoopThread::EventLoopThread() : loop_(NULL), exiting_(false), thread_(bind(&EventLoopThread::threadFunc, this), "EventLoopThread"), mutex_(), cond_(mutex_)
{
}

EventLoopThread::~EventLoopThread()
{
    exiting_ = true;
    if (loop_ != nullptr)
    {
        loop_->quit();
        thread_.join(); // 主线程阻塞等待子线程执行完毕后才执行下一个
    }
}
/*这段代码的主要目的是确保线程没有重复启动，并且等待线程中的 EventLoop 对象真正启动后才返回该对象，以便后续的操作可以安全地使用这个 EventLoop 对象。返回一个事件循环loop*/
EventLoop *EventLoopThread::startLoop()
{
    assert(!thread_.started()); // 确保没有启动
    thread_.start();
    {
        MutexLockGuard lock(mutex_);
        /*
        1.当条件 loop_ == NULL 满足时，线程会调用 cond_.wait() 进入等待状态。

        2.进入等待状态的同时，cond_.wait() 会释放它之前由 MutexLockGuard lock(mutex_); 所获取的 mutex_ 互斥锁。这个释放操作允许其他线程在必要时获取这个互斥锁并修改共享数据。

        3.等待条件变量的线程会一直阻塞在这里，直到有其他线程通过条件变量 cond_ 发送信号（通常是通过 cond_.notify() 或 cond_.notify_all() 发送），并且条件变量的条件为真。

        4.当其他线程发送了信号并满足了条件时，等待的线程会被唤醒，并重新获取 mutex_ 互斥锁。此时它会重新检查条件 loop_ == NULL，如果条件不满足，线程会继续执行下去。*/
        while (loop_ == nullptr)
        { /*使当前线程在条件变量上等待，并且在等待过程中会释放相关的互斥锁*/
            cond_.wait();
        }
    }
    return loop_;
}

/*把创建一个loop_，在构造函数中绑定到一个线程Thread*/
void EventLoopThread::threadFunc()
{
    EventLoop loop;
    {
        MutexLockGuard lock(mutex_);
        loop_ = &loop;
        cond_.notify();
    }
    loop.loop();
    loop_ = NULL;
}