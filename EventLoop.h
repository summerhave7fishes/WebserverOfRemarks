#pragma once
#include<iostream>
#include <functional>
#include"CurrentThread.h"
#include<assert.h>
#include"Channel.h"
#include <memory>
#include"Epoll.h"
#include"MutexLock.h"
class EventLoop{
    public:
    typedef std::function<void()> Functor;
    /*// 初始化poller, event_fd，给 event_fd 注册到 epoll 中并注册其事件处理回调*/
    ~EventLoop();
    EventLoop();
    void loop();
    void quit();
    void runInLoop(Functor &&cb);
    void queueInLoop(Functor &&cb);
    bool isInLoopThread() const{return threadId_==CurrentThread::tid();}

    void assertInLoopThread(){
        assert(isInLoopThread());
    }

    void shutdowm(std::shared_ptr<Channel> channel){
        shutDownWR(channel->getFd());
    }

    void removeFromPoller(std::shared_ptr<Channel> channel)
    {
        poller_->epoll_del(channel);
    }

    void updatePoller(std::shared_ptr<Channel> channel,int timeout=0)
    {
        poller_->epoll_mod(channel,timeout);
    }
    
    void addToPoller(shared_ptr<Channel> channel,int timeout=0)
    {
        poller_->epoll_add(channel,timeout);
    }

    

    private:
    bool looping_;//线程是否循环
     const pid_t threadId_;
    std::shared_ptr<Epoll> poller_;
    int wakeupFd_;//用于唤醒EventLoop
    bool quit_;//是否终止循环
    bool eventHandling_;//事件是否在处理
    mutable MutexLock mutex_;//你可以在常量成员函数中使用互斥锁，而不违反常量成员函数不修改对象状态的原则。
    std::vector<Functor> pendingFunctors_;
    bool callingPendingFunctors_;
    std::shared_ptr<Channel> pwakeupChannel_;//唤醒事件循环事件（EventLoop事件循环）

    void wakeup();
    void handleRead();
    void doPendingFunctors();
    void handleConn();
};
