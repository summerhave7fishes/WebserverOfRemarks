#pragma once
#include <vector>
#include <functional>
#include <string>
#include <memory>
#include "noncopyable.h"
#include "Thread.h"
#include "MutexLock.h"
#include "Condition.h"
#include "LogStream.h"
#include "CountDownLatch.h"
#include"LogFile.h"
class AsyncLogging : noncopyable
{
public:
    AsyncLogging(const std::string basename, int flushInterval = 2);
    ~AsyncLogging()
    {
        if (running_)
            stop();
    }

    void append(const char *logline, int len);

    void start()
    {
        running_ = true;
        thread_.start();
        latch_.wait();
    }

    void stop()
    {
        running_ = false;
        cond_.notify();
        thread_.join();
    }

private:
    void threadFunc();
    typedef FixedBuffer<kLargeBuffer> Buffer;
    // 可以容纳多个指向 Buffer 类型对象的智能指针。
    typedef std::vector<std::shared_ptr<Buffer>> BufferVector;
    typedef std::shared_ptr<Buffer> BufferPtr;
    bool running_;
    Thread thread_;
    MutexLock mutex_;
    Condition cond_;
    CountDownLatch latch_;
    const int flushInterval_;
    std::string basename_;
    BufferPtr currentBuffer_;
    BufferPtr nextBuffer_;
    BufferVector buffers_;
};