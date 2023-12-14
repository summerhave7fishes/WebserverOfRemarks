#pragma once
#include <sys/epoll.h>
#include <vector>
#include <memory>
#include"Channel.h"
class Epoll
{
public:
    Epoll();
    ~Epoll();
    void epoll_add(SP_Channel request, int timeout);
    void epoll_mod(SP_Channel request, int timeout);
    void epoll_del(SP_Channel request);

    std::vector<std::shared_ptr<Channel>> poll();
    std::vector<std::shared_ptr<Channel>> getEventsRequest(int events_num);

    int add_timer(std::shared_ptr<Channel> request_data, int timeout);
    int getEpollFd() { return epollFd_; }
    void handleExpired();

private:
    static const int MAXFDS = 100000;
    int epollFd_;//构造函数中初始化
    // 返回的epoll时间数组
    std::vector<epoll_event> events_;//构造函数中初始化
    std::shared_ptr<Channel> fd2chan_[MAXFDS];
    //一个数组，里面是指向HttpData的指针，一个Channel绑定一个HttpData
    std::shared_ptr<HttpData> fd2http_[MAXFDS];
    TimerManager timerManager_;
};