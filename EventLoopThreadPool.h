#pragma once
#include "noncopyable.h"
#include "EventLoopThread.h"
#include "Logging.h"
#include <vector>
#include <memory>
class EventLoopThreadPool : noncopyable
{
public:
    // 这个baseloop是主事件循环
    EventLoopThreadPool(EventLoop *baseLoop, int numThreads);
    ~EventLoopThreadPool() { LOG << "~EventLoopThreadPool()"; };
    void start();
    EventLoop *getNextLoop();

private:
    EventLoop *baseLoop_;
    bool started_;
    int numThreads_;
    int next_;
    // threads数组装的是EventLoopThread事件循环线程
    std::vector<std::shared_ptr<EventLoopThread>> threads_;
    // loops_装的是事件循环
    std::vector<EventLoop *> loops_;
};