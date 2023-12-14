#pragma once
#include "EventLoopThreadPool.h"
#include "EventLoop.h"
#include "Channel.h"

class Server
{
public:
    Server(EventLoop *loop, int threadNum, int port);
    ~Server() {}
    EventLoop *getLoop() const { return loop_; }
    void start();
    void handNewConn();
    void handThisConn() { loop_->updatePoller(acceptChannel_); };

private:
    EventLoop *loop_;//主循环
    int threadNum_;
    /*1.传入baseloop，这个是主循环，这个主循环一直接受服务，然后创建子线程
      2.传入numThreads，一共传入多少个线程*/
    bool started_;
    int port_;
    int listenFd_;
    std::unique_ptr<EventLoopThreadPool> eventLoopThreadPool_;
    std::shared_ptr<Channel> acceptChannel_;
    static const int MAXFDS = 100000;
};