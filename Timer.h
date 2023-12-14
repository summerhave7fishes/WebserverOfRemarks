#include <unistd.h>
#include <deque>
#include <memory>
#include <queue>
#include "HttpData.h"

class HttpData;

/*Timer.h文件里主要是定义了三个功能
1.时间结点结构TimerNode
1.1更新时间函数updata()
1.2时间结点是否有效
1.3清除时间结点clearReq()
1.4删除setDelete()
1.5是否删除isDelete()
1.6得到到期时间getExpTime()

2.时间结点的比较struct TimerCmp
3.时间结点的管理类型TimerManager
3.1给HttpData增加时间结点类 addTimer()
3.2处理超期时间 handleExpiredEvent()


一个TimerNode要和一个HttpData类绑定，同时一个HttpData类里面也有一个TiemrNode去进行超时删除操作
*/
class TimerNode
{
public:
    // 每一个HttpData都有一个timernode的对象去进行事件超时的管理
    TimerNode(std::shared_ptr<HttpData> requestDate, int timeout);
    ~TimerNode();
    TimerNode(TimerNode &tn);
    void update(int timeout);
    bool isValid();
    void clearReq();
    // delete
    void setDeleted() { deleted_ = true; }
    // 判断是否delete,这里设置const是不能够改变成员变量的值
    bool isDeleted() const
    {
        return deleted_;
    }
    // 得到到期时间
    size_t getExpTime() const { return expiredTime_; }

private:
    bool deleted_;       // 判断这个定时器是否删掉的标识符
    size_t expiredTime_; // 到期时间
    std::shared_ptr<HttpData> SPHttpData;
};
// 一个结构体，这个结构体是去比较a和b的到期时间，使用这两个参数中的 getExpTime() 函数来比较这两个 TimerNode 共享指针的到期时间。如果 a 的到期时间大于 b 的到期时间，则返回 true；否则返回 false。
struct TimerCmp
{
    bool operator()(std::shared_ptr<TimerNode> &a, std::shared_ptr<TimerNode> &b)
    {
        return a->getExpTime() > b->getExpTime();
    }
};

class TimerManager
{
public:
    TimerManager();
    ~TimerManager();
    void addTimer(std::shared_ptr<HttpData> SPHttpData, int timeout);
    void handleExpiredEvent();

private:
    typedef std::shared_ptr<TimerNode> SPTimerNode;
    /*这行代码声明了一个名为timerNodeQueue的优先队列，其元素类型为SPTimerNode。该优先队列使用std::deque作为其底层容器，并使用自定义的比较函数TimerCmp来进行元素的比较。*/
    std::priority_queue<SPTimerNode, std::deque<SPTimerNode>, TimerCmp> timerNodeQueue;
};