#pragma once
#include <errno.h>
#include "./base/MutexLock.h"
#include "noncopyable.h"

class Condition : noncopyable
{
public:
    /*初始化一个条件变量，同时将传入一个mutex保存到成员函数中*/
    explicit Condition(MutexLock &_mutex) : mutex(_mutex)
    {
        pthread_cond_init(&cond, NULL);
    }
    ~Condition() { pthread_cond_destroy(&cond); }
    /*它的作用是让当前线程在等待某个条件变量满足之前进入睡眠状态，并释放其所持有的互斥锁，以允许其他线程可以访问共享资源。当条件变量接收到信号后，线程会重新获得互斥锁并从 pthread_cond_wait 函数返回*/
    void wait() { pthread_cond_wait(&cond, mutex.get()); }
    /*会唤醒正在等待特定条件变量的一个线程。如果有多个线程在等待这个条件变量，那么通常只会有一个线程被唤醒并继续执行。被唤醒的线程会重新获得之前等待期间所释放的互斥锁，并从 pthread_cond_wait 函数的调用处返回，然后它可以继续执行后续的操作。*/
    void notify() { pthread_cond_signal(&cond); }

    void notifyAll() { pthread_cond_broadcast(&cond); }
    /*等待特定的条件变量，在等待一定时间后如果条件变量未满足则返回超时错误码。
    使用 clock_gettime 函数获取当前的系统时间，将其存储在 abstime 结构体中。
将 abstime 中的秒数增加了 seconds 秒，得到一个新的绝对时间。
使用 pthread_cond_timedwait 函数来等待条件变量 cond，在等待过程中会使用 abstime 作为超时时间。如果条件变量在超时时间内未满足，则会返回 ETIMEDOUT 错误码，否则会返回其他适当的值。*/
    bool waitForSeconds(int seconds)
    {
        struct timespec abstime;
        clock_gettime(CLOCK_REALTIME, &abstime);
        abstime.tv_sec += static_cast<time_t>(seconds);
        return ETIMEDOUT == pthread_cond_timedwait(&cond, mutex.get(), &abstime);
    }

private:
    MutexLock &mutex;
    pthread_cond_t cond;
};
