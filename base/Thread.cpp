#include "Thread.h"
#include "CurrentThread.h"
#include "CountDownLatch.h"
#include <sys/syscall.h>
#include <unistd.h>
#include <sys/prctl.h>
#include <assert.h>
#include <string.h>
namespace CurrentThread
{
    __thread int t_cacheTid = 0;
    __thread char t_tidString[32];
    __thread int t_tidStringLength = 6;
    __thread const char *t_threadName = "default";
}

pid_t gettid()
{
    return static_cast<pid_t>(::syscall(SYS_gettid));
}

/*将 t_cacheTid 格式化为一个 5 位的带符号的十进制整数，并将结果以字符串的形式存储在 t_tidString 中,返回字符串长度。
系统返回的tid存到t_cacheTid中，tid以字符串的形式储存在t_tidString中，同时，tid的长度存在t_tidStringLength*/
void CurrentThread::cacheTid()
{
    if (t_cachedTid == 0)
    {
        t_cachedTid = gettid();
        t_tidStringLength = snprintf(t_tidString, sizeof t_tidString, "%5d", t_cacheTid);
    }
}

// 为了在线程中保留name,tid,threadfunc,latch这些数据
struct ThreadData
{
    typedef Thread::ThreadFunc ThreadFunc;
    ThreadFunc func_;
    std::string name_;
    pid_t *tid_;
    CountDownLatch *latch_;

    ThreadData(const ThreadFunc &func, const std::string &name, pid_t *tid, CountDownLatch *latch) : func_(func), name_(name), tid_(tid), latch_(latch) {}

    void runInThread()
    {
        *tid_ = CurrentThread::tid();
        tid_ = NULL;
        /*调用是为了通知其他等待线程，当前线程已经准备就绪并即将开始执行。当其他等待线程收到通知后，它们将开始执行其相应的操作。而当前线程将继续执行 func_() 函数中的代码，直到该函数执行完成。*/
        latch_->countDown();
        latch_ = NULL;
        CurrentThread::t_threadName = name_.empty() ? "Thread" : name_.c_str();
        /*prctl 是一个系统调用，用于设置进程的运行时属性。PR_SET_NAME 是 prctl 的一个参数，用于设置进程的名称。CurrentThread::t_threadName 是一个表示线程名称的字符串，用于将当前线程的名称设置为指定的名称。*/
        prctl(PR_SET_NAME, CurrentThread::t_threadName);
        func_();
        CurrentThread::t_threadName = "finished";
    }
};

void *startThread(void *obj)
{
    ThreadData *data = static_cast<ThreadData *>(obj);
    data->runInThread();
    delete data;
    return NULL;
}

Thread::Thread(const ThreadFunc &func, const std::string &n) : started_(false), joined_(false), pthreadId_(0), tid_(0), func_(func), name_(n), latch_(1)
{
    setDefaultName();
}

void Thread::setDefaultName()
{
    if (name_.empty())
    {
        char buf[32];
        snprintf(buf, sizeof buf, "Thread");
        name_ = buf;
    }
}
/*pthread_detach 函数会通知系统，线程的资源可以被回收并由系统自行处理。这意味着即使线程没有被 join，它的资源也不会被一直保留，因此不会出现资源泄露的问题。只有未被 join 的线程才会被分离，已经被 join 的线程不会被重复操作*/
Thread::~Thread()
{
    if (started_ && !joined_)
        pthread_detach(pthreadId_);
}

void Thread::start()
{
    assert(!started_);
    started_ = true;
    /*data里面的latch_会调用countDown()*/
    ThreadData *data = new ThreadData(func_, name_, &tid_, &latch_);
    /*在这里，&pthreadId_ 是一个指向线程标识符的指针，startThread 是一个函数，data 是传递给 startThread 函数的参数。*/
    if (pthread_create(&pthreadId_, NULL, &startThread, data))
    {
        started_ = false;
        delete data;
    }
    else
    {
        /*latch_.wait() 会阻塞当前线程，直到计数器的值变为 0。*/
        latch_.wait();
        assert(tid_ > 0);
    }
}
/*在这段代码中，Thread::join 函数用于等待线程的完成。它首先通过断言确保线程已经开始执行并且尚未加入（join）。然后，将joined_标志设置为true，以防止重复加入。最后，它调用 pthread_join 函数来等待线程完成并回收线程资源。函数返回线程的退出代码。*/
int Thread::join()
{
    assert(started_);
    assert(!joined_);
    joined_ = true;
    return pthread_join(pthreadId_, NULL);
}