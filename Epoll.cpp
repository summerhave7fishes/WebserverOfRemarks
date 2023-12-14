#include "Epoll.h"
#include <assert.h>
#include "base/Logging.h"

const int EVENTSNUM = 4096;
const int EPOLLWAIT_TIME = 10000;

Epoll::Epoll() : epollFd_(epoll_create1(EPOLL_CLOEXEC)), events_(EVENTSNUM)
{
    assert(epollFd_ > 0);
}

Epoll::~Epoll() {}

// 注册新描述符
void Epoll::epoll_add(SP_Channel request, int timeout)
{
    int fd = request->getFd();
    if (timeout > 0)
    {
        // 添加一个定时器
        add_timer(request, timeout);
        // 关联一个httpData的对象
        fd2http_[fd] = request->getHolder();
    }
    struct epoll_event event;
    /*当前的fd是什么events*/
    event.data.fd = fd;
    event.events = request->getEvents();
    request->EqualAndUpdateLastEvents();

    fd2chan_[fd] = request;
    if (epoll_ctl(epollFd_, EPOLL_CTL_ADD, fd, &event) < 0)
    {
        perror("epoll_add error");
        fd2chan_[fd].reset();
    }
}

void Epoll::epoll_del(SP_Channel request)
{
    int fd = request->getFd();
    struct epoll_event event;
    event.events = request->getEvents();
    if (epoll_ctl(epollFd_, EPOLL_CTL_DEL, fd, &event) < 0)
    {
        perror("epoll_del error");
    }
    fd2chan_[fd].reset();
    fd2http_[fd].reset();
}

// 返回活跃事件数
std::vector<SP_Channel> Epoll::poll()
{
    while (true)
    {
        /*epoll_wait会将就绪的事件从内核态拷贝到events_数组中，并返回就绪事件的数量。*/
        int event_count = epoll_wait(epollFd_, &*events_.begin(), events_.size(), EPOLLWAIT_TIME);
        if (event_count < 0)
            perror("epoll wait error");
        // 把得到的时间放入events_
        // req_data里面的每一个Channel都有一个从epoll中拿出来的事件events
        std::vector<SP_Channel> req_data = getEventsRequest(event_count);
        if (req_data.size() > 0)
            return req_data;
    }
}

// 超时处理,删掉delete掉TimerNode
void Epoll::handleExpired()
{
    timerManager_.handleExpiredEvent();
}

// 分发处理函数，核心
std::vector<SP_Channel> Epoll::getEventsRequest(int events_num)
{
    std::vector<SP_Channel> req_data;
    for (int i = 0; i < events_num; ++i)
    {
        int fd = events_[i].data.fd;
        SP_Channel cur_req = fd2chan_[fd];
        // 每一个Channel去设置一个events（读、写、连接）
        if (cur_req)
        {
            cur_req->setEvents(events_[i].events);
            cur_req->setEvents(0);
            req_data.push_back(cur_req);
        }
        else
        {
            LOG << "SP cur_req is invalid";
        }
    }
    return req_data;
}
