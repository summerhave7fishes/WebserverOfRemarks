#pragma once
#include "Channel.h"
#include <pthread.h>
#include <functional>
#include <memory>
#include <vector>

const int THREADPOOL_INVALID = -1;
const int THREADPOOL_LOCK_FAILURE = -2;
const int THREADPOOL_QUEUE_FULL = -3;
const int THREAQDPOOL_SHUTDOWN = -4;
const int THREADPOOL_THREAD_FAILURE = -5;
const int THREADPOOL_GRACEFUL = 1;

const int MAX_THREADS = 1024; // 存疑
const int MAX_QUEUE = 65535;

typedef enum
{
    immediate_shutdown = 1,
    graceful_shutdown = 2
} shutDownOption;
/*fun 存储的可调用对象接受一个 std::shared_ptr<void> 类型的参数，并且不返回任何值。在这里，arg 用于存储传递给 fun 对象的参数。。这意味着 ThreadPoolTask 结构体的实例可以存储一个函数，并在需要时将该函数作为任务提交到线程池执行，同时可以携带一个 void 类型的参数 arg。*/
struct ThreadPoolTask
{
    std::function<void(std::shared_ptr<void>)> fun;
    std::shared_ptr<void> args;
};

class ThreadPool
{
private:
    static pthread_mutex_t lock;
    static pthread_cond_t notify;

    static std::vector<pthread_t> threads;
    static std::vector<ThreadPoolTask> queue;
    static int thread_count; // 线程数量threads
    static int queue_size;
    static int head;
    static int tail;  // 最后一个，空的，将现在需要放进去的参数和函数放进去，然后将其转到next
    static int count; // queue的数量
    static int shutdown;
    static int started;

public:
/*创建_thread_count个线程，并且执行queue中的任务*/
    static int threadpool_creat(int _thread_count, int _queue_size);
    /*将任务加入queue中*/
    static int threadpool_add(std::shared_ptr<void> arg, std::function<void(std::shared_ptr<void>)> func);
    static int threadpool_destroy(shutDownOption shutdown_option = graceful_shutdown);
    static int threadpool_free();
    /*作为线程函数的入口点，在启动线程时作为参数传递给 pthread_create() 或类似的线程创建函数。循环将queue中的任务取出来，放入线程中执行*/
    static void *threadpool_thread(void *arg);
};