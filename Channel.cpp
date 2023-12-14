#include "Channel.h"

using namespace std;

Channel::Channel(EventLoop *loop) : loop_(loop), events_(0), lastEvents_(0), fd_(0) {}

Channel::Channel(EventLoop *loop, int fd) : loop_(loop), fd_(fd), events_(0), lastEvents_(0) {}

Channel::~Channel()
{
}

int Channel::getFd() { return fd_; }

void Channel::setFd(int fd)
{
    fd_ = fd;
}

/*如果 revents_ 中的某些标志位被设置，包括 EPOLLIN、EPOLLPRI 或 EPOLLRDHUP 中的任意一个或多个，那么就调用 handleRead() 函数。handleRead() 函数的逻辑是检查是否存在一个回调函数 readHandler_，如果存在，则调用它*/
void Channel::handleRead()
{
    if (readHandler_)
    {
        readHandler_();
    }
}

void Channel::handleWrite()
{
    if (writeHandler_)
    {
        writeHandler_();
    }
}

void Channel::handleConn()
{
    if (connHandler_)
    {
        connHandler_();
    }
}