#include "Timer.h"
#include <sys/time.h>
#include <unistd.h>
#include <queue>

TimerNode::TimerNode(std::shared_ptr<HttpData> requestDate, int timeout) : deleted_(false), SPHttpData(requestDate)
{
    struct timeval now;
    /*gettimeofday(&now,NULL) 是一个用于获取当前时间的函数调用。其中 &now 是一个指向 timeval 结构的指针，NULL 则是一个时区结构的指针。gettimeofday 函数用于获取当前的时间和日期，包括秒数和微秒数。*/
    gettimeofday(&now, NULL);
    /*expiredTime_ 被设置为一个相对时间点，即从当前时间开始经过了 timeout 毫秒之后的时间点。
    首先，它从 now 变量中获取了当前时间的秒数 tv_sec 和微秒数 tv_usec。然后它对当前秒数取模 10000，这样做可能是为了防止超出范围。接着将取模后的秒数乘以 1000，这样得到的是毫秒数。之后，将微秒数除以 1000，这样得到的也是毫秒数。*/
    expiredTime_ = (((now.tv_sec % 10000) * 1000) + (now.tv_usec / 1000)) + timeout;
}

/*这个地方析构TimerNode的时候不会让HttpData删掉，但是会让channel_从epoll中删掉*/
TimerNode::~TimerNode()
{
    // 这个handleClose()函数的实现在HttpData中，主要有三个实现：连接状态改为断开连接，确保在 handleClose 方法执行期间，HttpData 对象不会被意外释放以及移除loop中的channel_
    if (SPHttpData)
        SPHttpData->handleClose();
}

TimerNode::TimerNode(TimerNode &tn) : SPHttpData(tn.SPHttpData), expiredTime_(0)
{
}

/*得到的结果就是新的到期时间 expiredTime_*/
void TimerNode::update(int timeout)
{
    struct timeval now;
    gettimeofday(&now, NULL);
    expiredTime_ = ((now.tv_sec % 10000) * 1000 + (now.tv_usec / 1000)) + timeout;
}

/*查看时间是否有效，逻辑如下：
1.得到现在的时间tmp
2.如果现在<到期时间 return true
3.如果现在>到期时间  return false*/
bool TimerNode::isValid()
{
    struct timeval now;
    gettimeofday(&now, NULL);
    size_t temp = ((now.tv_sec % 10000) * 1000 + (now.tv_usec / 1000));
    if (temp < expiredTime_)
    {
        return true;
    }
    else
    {
        this->setDeleted();
        return false;
    }
}

/*清除关联的 SPHttpData对象，并设置 TimerNode 的状态为已删除。*/
void TimerNode::clearReq()
{
    /*共享指针清楚，如果是没有其他地方调用SPHttpData，那么就析构*/
    SPHttpData.reset();
    this->setDeleted();
}

TimerManager::TimerManager()
{
}

TimerManager::~TimerManager() {}

void TimerManager::addTimer(std::shared_ptr<HttpData> SPHttpData, int timeout)
{
    // 先构建一个timerNode,加入优先级队列，然后与HttpData关联
    SPTimerNode new_node = std::make_shared<TimerNode>(SPHttpData, timeout);
    timerNodeQueue.push(new_node);
    SPHttpData->linkTimer(new_node);
}

/*1.删除标识为delete的，如果非第一个被标志为delete，不会马上删除，会等到delete这个超时了之后再删除。
原因：第一个好处是不需要遍历优先队列，省时，第二个好处是给超时时间一个容忍的时间，就是设定的超时时间是删除的下限(并不是一到超时时间就立即删除)，如果监听的请求在超时后的下一次请求中又一次出现了，
就不用再重新申请RequestData节点了，这样可以继续重复利用前面的RequestData，减少了一次delete和一次new的时间
2.删除超时的
*/
void TimerManager::handleExpiredEvent()
{
    while (!timerNodeQueue.empty())
    {
        SPTimerNode ptimer_now = timerNodeQueue.top();
        if (ptimer_now->isDeleted())
        {
            timerNodeQueue.pop();
        }
        else if (ptimer_now->isValid() == false)
        {
            timerNodeQueue.pop();
        }
        else
        {
            break;
        }
    }
}
