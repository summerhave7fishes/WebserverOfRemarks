#include"EventLoopThreadPool.h"
EventLoopThreadPool::EventLoopThreadPool(EventLoop *baseloop,int numThreads):baseLoop_(baseloop),started_(false),numThreads_(numThreads),next_(0){
    if(numThreads_<=0)
    {
        LOG<<"numThreads_<=0";
        abort();
    }
}


void EventLoopThreadPool::start(){
    //主循环是否已经开始
    baseLoop_->assertInLoopThread();
    started_=true;
    for(int i=0;i<numThreads_;++i)
    {
        //创建numThreads个事件循环线程，将线程放入threads_数组，事件放进事件循环数组loops_
        std::shared_ptr<EventLoopThread> t(new EventLoopThread());
        threads_.push_back(t);
        loops_.push_back(t->startLoop());

    }
    
}


EventLoop *EventLoopThreadPool::getNextLoop()
{
    baseLoop_->assertInLoopThread();
    assert(started_);
    EventLoop *loop=baseLoop_;
    if(!loops_.empty())
    {
        loop=loops_[next_];
        next_=(next_+1)%numThreads_;
    }
    return loop;
}