#pragma once
#include <pthread.h>
#include <functional>
#include "noncopyable.h"
#include "CountDownLatch.h"
#include <string.h>

class Thread : noncopyable
{
public:
    typedef std::function<void()> ThreadFunc;
    // 构造函数，参数1是一个线程入口，函数2是线程名称
    explicit Thread(const ThreadFunc &, const std::string &name = std::string());
    ~Thread();
    void start();
    int join();
    bool started() const { return started_; }
    pid_t tid() const { return tid_; }
    const std::string &name() const { return name_; }

private:
    void setDefaultName();
    bool joined_;
    pthread_t pthreadId_;
    bool started_;
    pid_t tid_;
    std::string name_;
    CountDownLatch latch_;
    ThreadFunc func_;
};