#pragma once
#include <functional>
#include"EventLoop.h"
#include"base/Condition.h"
#include"noncopyable.h"
#include"Thread.h"
#include"MutexLock.h"
using namespace std;
class EventLoopThread:noncopyable{
public:
    EventLoopThread();
    ~EventLoopThread();
    EventLoop *startLoop();

    private:
    void threadFunc();
    EventLoop *loop_;
    bool exiting_;
    Thread thread_;
    MutexLock mutex_;
    Condition cond_;

};