#pragma once
#include "Condition.h"
#include "./base/MutexLock.h"
#include "noncopyable.h"

// CountDownLatch的主要作用是确保Thread中传进去的func真的启动了以后
// 外层的start才返回

class CountDownLatch : noncopyable
{
public:
    explicit CountDownLatch(int count);
    /*一个线程启动的时候调用wait()，如果count_≠0，则一直等待*/
    void wait();
    /*其他线程调用countDown，当count_=0的时候，发送notifyAll，其他线程就可以启动*/
    void countDown();

private:
    mutable MutexLock mutex_;
    Condition condition_;
    int count_;
};