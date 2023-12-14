#include "EventLoop.h"
#include <sys/eventfd.h>
#include "base/Logging.h"
using namespace std;
__thread EventLoop *t_loopInThisThread = 0; // 每个线程可以有自己独立的 EventLoop 实例。每个线程都有其自己独立的 t_loopInThisThread 变量，并且在线程内部可以使用这个变量来存储和访问特定于该线程的 EventLoop 对象的地址。

int createEventfd()
{
  /*而是一个用于线程间通信的文件描述符。你可以使用它来实现线程间的事件通知*/
  int evtfd = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
  if (evtfd < 0)
  {
    LOG << "Failed in eventfd";
    abort();
  }
  return evtfd;
}

EventLoop::EventLoop() : looping_(false), poller_(new Epoll()), wakeupFd_(createEventfd()), quit_(false), eventHandling_(false), callingPendingFunctors_(false), threadId_(CurrentThread::tid()), pwakeupChannel_(new Channel(this, wakeupFd_))
{

  if (t_loopInThisThread)
  {
  }
  else
  {
    t_loopInThisThread = this;
  }
  /*EventLoop与一个Thread绑定在一起->EventLoop中绑定Channel事件(读与连接)，将Channel中的读写事件加入Epoll*/
  pwakeupChannel_->setEvents(EPOLLIN | EPOLLET);
  pwakeupChannel_->setReadHandler(bind(&EventLoop::handleRead, this));
  pwakeupChannel_->setConnHandler(bind(&EventLoop::handleConn, this));
  poller_->epoll_add(pwakeupChannel_, 0);
}

void EventLoop::handleConn()
{
  updatePoller(pwakeupChannel_, 0);
}

EventLoop::~EventLoop()
{
  // 关掉唤醒EventLoop的Fd
  close(wakeupFd_);
  t_loopInThisThread = nullptr;
}
/*这段代码的目的是往 wakeupFd_ 文件描述符中写入一个64位的值 1，用来唤醒相应的事件循环，然后检查写入操作是否成功，如果写入的字节数不是预期的 8 个字节，就会输出一条日志信息来表示写入字节数不正确。*/
void EventLoop::wakeup()
{
  uint64_t one = 1;
  ssize_t n = writen(wakeupFd_, (char *)(&one), sizeof one);
  if (n != sizeof one)
  {
    LOG << "EventLoop::wakeup() writes" << n << "bytes instead of 8";
  }
}

/*读取EventLoop是否被唤醒，wakeupFd_是否为1，如果唤醒了就将handleRead事件绑定*/
void EventLoop::handleRead()
{
  uint32_t one = 1;
  ssize_t n = readn(wakeupFd_, &one, sizeof one);
  if (n != sizeof one)
  {
    LOG << "EventLoop::handleRead() reads " << n << " bytes instead of 8";
  }

  pwakeupChannel_->setEvents(EPOLLIN | EPOLLET);
}

/*意思是执行cb这个任务，如果就是在EventLoop线程里面，就直接执行cb回调函数，否则放入pendingFunctors_队列中*/
void EventLoop::runInLoop(Functor &&cb)
{
  if (isInLoopThread())
  {
    cb();
  }
  else
    queueInLoop(std::move(cb));
}

void EventLoop::queueInLoop(Functor &&cb)
{
  MutexLockGuard lock(mutex_);
  pendingFunctors_.emplace_back(std::move(cb));
  if (!isInLoopThread() || callingPendingFunctors_)
    wakeup();
}

void EventLoop::loop()
{
  assert(!looping_);
  assert(isInLoopThread());
  looping_ = true;
  quit_ = false;
  std::vector<SP_Channel> ret;
  while (!quit_)
  {
    ret.clear();
    ret = poller_->poll();
    eventHandling_ = true;
    for (auto &it : ret)
    {
      eventHandling_ = false;
      /*处理待处理任务队列里面的待处理任务pendingFunctors_*/
      doPendingFunctors();
      poller_->handleExpired();
    }
  }
  looping_ = false;
}

/*
1.std::vector<Functor> functors;：首先，创建一个用于存储待处理任务（Functors）的向量（vector）functors。

2.callingPendingFunctors_ = true;：设置一个标志位callingPendingFunctors_为true，表示当前正在执行待处理任务，这是为了避免在执行期间再次调用doPendingFunctors。

3.使用互斥锁：在代码块内部，通过互斥锁（Mutex）来保护对pendingFunctors_的访问。pendingFunctors_通常是一个存储待处理任务的队列。

4.functors.swap(pendingFunctors_);：通过调用swap函数，将pendingFunctors_中的待处理任务全部移动到functors中，这样可以减少锁的持有时间，提高性能。此时，pendingFunctors_应该是一个空的队列。

5.遍历functors并执行：遍历functors，并对其中的每个Functor（一般是函数对象或Lambda函数）进行执行。这些Functors通常是在事件循环中被异步添加的任务。

6.callingPendingFunctors_ = false;：设置callingPendingFunctors_为false，表示待处理任务执行完毕，允许再次调用doPendingFunctors。*/
void EventLoop::doPendingFunctors()
{
  std::vector<Functor> functors;
  callingPendingFunctors_ = true;
  {
    MutexLockGuard lock(mutex_);
    functors.swap(pendingFunctors_);
  }
  for (size_t i = 0; i < functors.size(); ++i)
    functors[i]();
  callingPendingFunctors_ = false;
}

void EventLoop::quit()
{
  quit_ = true;
  if (!isInLoopThread())
  {
    wakeup();
  }
}