#pragma once
#include <sys/epoll.h>

#include <memory>
#include <functional>

#include "Timer.h"
class EventLoop;
class HttpData;

class Channel
{
private:
    /*
    std::function 是一个通用的函数包装器，它可以用于封装可以以函数调用方式执行的任何可调用实体（例如函数、lambda 表达式、函数对象等）。std::function<void()> 是一个表示不接受任何参数并且不返回任何值的函数的类型。

    CallBack 的类型别名，它代表了一个可以在不接受任何参数的情况下调用的回调函数。
    */
    typedef std::function<void()> CallBack;
    EventLoop *loop_;
    int fd_;
    __uint32_t events_;
    __uint32_t revents_;
    __uint32_t lastEvents_;

    // 方便找到上层持有该Channel的对象
    std::weak_ptr<HttpData> holder_;

private:
    int parse_URI();
    int parse_Headers();
    int analysisRequest();

    CallBack readHandler_;
    CallBack writeHandler_;
    CallBack errorHandler_;
    CallBack connHandler_;

public:
    Channel(EventLoop *loop);
    Channel(EventLoop *loop, int fd);
    ~Channel();

    int getFd();
    void setFd(int fd);
    /*一个HttpData绑定一个Channel*/
    void setHolder(std::shared_ptr<HttpData> holder)
    {
        holder_ = holder;
    }

    // 获得Channel绑定的HttpData
    std::shared_ptr<HttpData> getHolder()
    {
        std::shared_ptr<HttpData> ret(holder_.lock());
        return ret;
    }

    /*设置读回调*/
    /*&&右值引用类型，表示readHandler是右值引用类型，，使用右值引用的主要目的是实现完美转发。通过将参数声明为右值引用，可以在函数内部将其转发到另一个函数，同时保持其值类别。这意味着，如果传递给 setReadHandler 的参数本身是右值，那么它将被作为右值转发到 readHandler_，而不会触发额外的拷贝操作。*/

    //这一部分会在HttpData类中的构造函数中用到，也就是在HttpData的构造函数中，将Channel的读时间、写事件回调绑定HttpData类中具体的读事件处理、写事件处理
    void setReadHandler(CallBack &&readHandler)
    {
        readHandler_ = readHandler;
    }

    void setWriteHandler(CallBack &&writeHandler)
    {
        writeHandler_ = writeHandler;
    }

    void setErrorHandler(CallBack &&errorHandler)
    {
        errorHandler_ = errorHandler;
    }

    void setConnHandler(CallBack &&connHandler)
    {
        connHandler_ = connHandler;
    }

/*这个函数在EventLoop的loop()函数去调用，loop函数得到一个vector<shared_ptr<Channel>> ret数组，然后里面的元素shared_ptr<Channel> channel_去调用handleEvents()函数，*/
    void handleEvents()
    {
        /*events_ 是一个标志位，用于表示事件类型或状态。在 handleEvents 函数中，events_ = 0 用于将 events_ 变量重置为初始状态。这通常用于清除先前的事件标志，以便在处理新的事件时始终从一个干净的状态开始。events_ 被用于确定当前的事件类型，而将其设置为 0 则意味着在处理完当前事件后，程序已经清除了所有的事件标志，为下一次事件处理做好了准备。*/
        events_ = 0;
        // EPOLLHUP挂起且没有读事件（EPOLLIN）
        if ((revents_ & EPOLLHUP) && !(revents_ & EPOLLIN))
        {
            events_ = 0;
            return;
        }
        /*如果errorHandler_这个函数存在，那么调用回调函数errorHandler_()
errorHandler_ 是一个 std::function 类型的对象，它可以存储可以通过函数调用方式执行的任何可调用实体，比如函数、lambda 表达式、函数对象等.在这里，errorHandler_() 是调用了 errorHandler_ 所指向的可调用实体。由于 errorHandler_ 是一个 std::function 对象，它可以在运行时存储任何可调用实体，并且可以通过 operator() 来调用它所持有的实体。
例子如下：
#include <iostream>
#include <functional>

// 定义回调函数类型别名
typedef std::function<void()> Callback;

// 事件处理函数
void eventHandler(Callback callback) {
std::cout << "Performing some event handling..." << std::endl;
callback();  // 调用回调函数
}

// 示例回调函数
void errorHandler() {
std::cout << "Error occurred! Handling error..." << std::endl;
}

int main() {
// 将错误处理函数作为回调函数传递给事件处理函数
eventHandler(errorHandler);

return 0;
}
输出：
Performing some event handling...
Error occurred! Handling error...
*/
        if (revents_ & EPOLLERR)
        {

            if (errorHandler_)
                errorHandler_();
            events_ = 0;
            return;
        }
        /*这意味着如果存在可读事件、紧急数据可读事件或连接关闭事件中的任何一种，就会调用 handleRead() 函数。*/
        if (revents_ & (EPOLLIN | EPOLLPRI | EPOLLRDHUP))
        {
            //调用handleRead()里面的回调函数readHandler_
            handleRead();
        }
        if (revents_ & EPOLLOUT)
        {
            handleWrite();
        }
        handleConn();
    }

    void handleRead();
    void handleWrite();
    /*在HttpData中去调用这个函数*/
    void handleError(int fd, int err_unm, std::string short_msg);
    void handleConn();

    void setRevents(__uint32_t ev) { revents_ = ev; }
    void setEvents(__uint32_t ev)
    {
        events_ = ev;
    }
    /*在这个示例中，getEvents() 返回了一个对私有成员变量 events_ 的引用，允许我们直接通过 Channel_.getEvents() 来访问和修改 events_ 的值。
    请注意，使用引用返回可能会导致潜在的意外行为，因为它允许外部代码修改私有成员。因此，在使用引用返回时，请务必谨慎处理。*/
    __uint32_t &getEvents() { return events_; }

    /*这个函数的目的是比较上一次记录的事件（lastEvents_）和当前的事件（events_）是否相等。它首先将比较结果存储在 ret 中，然后更新 lastEvents_ 为当前的 events_ 值，最后返回比较结果。*/
    bool EqualAndUpdateLastEvents()
    {
        bool ret = (lastEvents_ == events_);
        lastEvents_ = events_;
        return ret;
    }

    __uint32_t getLastEvents()
    {
        return lastEvents_;
    }
};

typedef std::shared_ptr<Channel> SP_Channel;