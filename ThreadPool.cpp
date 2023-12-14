#include "ThreadPool.h"

// 静态变量类外初始化
pthread_mutex_t ThreadPool::lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t ThreadPool::notify = PTHREAD_COND_INITIALIZER;
std::vector<pthread_t> ThreadPool::threads;
std::vector<ThreadPoolTask> ThreadPool::queue;

int ThreadPool::thread_count = 0;
int ThreadPool::queue_size = 0;
int ThreadPool::head = 0;
int ThreadPool::tail = 0;
int ThreadPool::count = 0;//任务队列里面目前有的任务数
int ThreadPool::shutdown = 0;
int ThreadPool::started = 0;//记录开启了多少工作线程，加入在threadpool_create里面开启了N个，在取出任务里面就要减去N个

/*创建_thread_count这么多个线程，放入threads数组中，并且执行queue中的任务*/
int ThreadPool::threadpool_creat(int _thread_count, int _queue_size)
{
    /*do 后面的大括号内可能包含一系列的代码，但由于 while(false); 的条件永远为 false，因此这个循环只会执行一次。它实际上并不是一个真正的循环，而是一个用于封装一段代码的技巧。*/
    bool err = false;
    do
    { /*这个代码段的目的是确保线程数量和队列大小在合理的范围内，如果不在合理范围内，则将其设置为默认值。*/
        if (_thread_count <= 0 || _thread_count > MAX_THREADS || _queue_size <= 0 || _queue_size > MAX_QUEUE)
        {
            _thread_count = 4;
            _queue_size = 1024;
        }
        thread_count = 0;
        queue_size = _queue_size;
        head = tail = count = 0;
        shutdown = started = 0;

        threads.resize(_thread_count);
        queue.resize(_queue_size);
        /*开始工作线程*/
        for (int i = 0; i < _thread_count; i++)
        {
            /*&threads[i]：表示线程标识符，用于存储新创建的线程的标识。
            NULL：是用于设置线程属性的参数，这里使用 NULL 表示使用默认的线程属性。
            threadpool_thread：是线程函数的指针，即将要在新线程中执行的函数。
            (void*)(0)：是传递给线程函数 threadpool_thread 的参数。在这里，传递了一个值为 0 的指针类型参数。*/
            if (pthread_create(&threads[i], NULL, threadpool_thread, (void *)(0)) != 0)
            {
                return -1;
            }
            ++thread_count;
            ++started;
        }

    } while (false);
}

/*给循环队列queue中加入task任务*/
int ThreadPool::threadpool_add(std::shared_ptr<void> args, std::function<void(std::shared_ptr<void>)> fun)
{
    int next, err = 0;
    /*如果该函数成功获取到互斥锁并成功加锁，则返回值为 0,所以不为0就是加锁失败*/
    if (pthread_mutex_lock(&lock) != 0)
        return THREADPOOL_LOCK_FAILURE;
    do
    {
        /*循环队列（Circular Queue）中计算下一个位置的公式。当 tail + 1 的值超过队列大小时，取余数操作可以使其在 0 到 queue_size - 1 的范围内循环。*/
        next = (tail + 1) % queue_size;
        // 队列满了
        if (count == queue_size)
        {
            err = THREADPOOL_QUEUE_FULL;
            break;
        }
        // 已关闭
        if (shutdown)
        {
            err = THREAQDPOOL_SHUTDOWN;
            break;
        }

        queue[tail].fun = fun;
        queue[tail].args = args;
        tail = next;
        ++count;
        /*发送一个信号给与条件变量 notify 相关联的等待线程中的一个*/
        if (pthread_cond_signal(&notify) != 0)
        {
            err = THREADPOOL_LOCK_FAILURE;
            break;
        }

    } while (false);
    //解锁
    if (pthread_mutex_unlock(&lock) != 0)
    {
        err = THREADPOOL_LOCK_FAILURE;
        return err;
    }

}

/*释放线程，然后关掉线程池
1.上锁
2.指定关闭方式
3.利用条件变量通知与条件变量相关的线程，利用广播的方式并且解锁
4.for循环关掉线程
5.线程池free掉*/
int ThreadPool::threadpool_destroy(shutDownOption shutdown_option)
{
    printf("Thread pool destroy!\n");
    int i,err=0;
    if(pthread_mutex_lock(&lock)!=0)
    {
        return THREADPOOL_LOCK_FAILURE;
    }

    do{
        //如果已经shutdown了
        if(shutdown)
        {
            err=THREAQDPOOL_SHUTDOWN;
            break;
        }
        shutdown=shutdown_option;
        /*通知等待线程线程池即将关闭*/
        if((pthread_cond_broadcast(&notify)!=0)||(pthread_mutex_unlock(&lock)!=0)){
            err=THREADPOOL_LOCK_FAILURE;
            break;
        }
        //线程分离
        for(int i=0;i<thread_count;i++)
        {/*等待指定线程 threads[i] 的执行结束，确保主线程在其他线程结束后再继续执行后续逻辑*/
            if(pthread_join(threads[i],NULL)!=0)
            {
                err=THREADPOOL_THREAD_FAILURE;
            }
        }


    }while(false);

    if(!err)
    {
        threadpool_free();
    }
    return err;
}

/*毁掉lock与cond*/
int ThreadPool::threadpool_free(){
    //暂定，目前应该是在pthread_creat线程函数里++，然后在threadpool_thread函数里--
    if(started>0)
    {
        return -1;
    }

    pthread_mutex_lock(&lock);
    pthread_mutex_destroy(&lock);
    pthread_cond_destroy(&notify);
    return 0;

}

/*循环执行queue里面的任务，queue里面装的是任务，在*/
void *ThreadPool::threadpool_thread(void *args)
{
    while(true)
    {
        ThreadPoolTask task;
        pthread_mutex_lock(&lock);

        //当队列中没有任务，并且线程池并没有被关闭的时候,将当前线程置于等待状态，释放lock互斥锁，等待
        while((count==0)&&(!shutdown))
        {/*当前线程处于等待状态，并且会释放 lock 互斥锁。线程会一直阻塞在这里，直到其他线程通过 */
            pthread_cond_wait(&notify,&lock);
        }

        //如果线程池马上关闭或者graceful关闭且目前任务队列里没有任务要做的时候，直接跳出循环
        if((shutdown==immediate_shutdown)||((shutdown==graceful_shutdown)&&(count==0)))
        {
            break;
        }
        
        //取出任务队列queue里面的任务
        task.fun=queue[head].fun;
        task.args=queue[head].args;
        queue[head].fun=NULL;
        queue[head].args.reset();
        //head置为下一个
        head=(head+1)%queue_size;
        --count;
        pthread_mutex_unlock(&lock);
        //这行代码的含义是执行 task.fun 所指向的函数，并将 task.args 作为参数传递给这个函数。
        (task.fun,task.args);

    }
    --started;
   
    pthread_mutex_unlock(&lock);
    printf("This threadpool thread finishs!\n");
    /*用于终止当前线程。调用该函数将使线程立即退出，不会执行后续的代码，相当于中断执行，但不会影响其他线程的执行。*/
    pthread_exit(NULL);
    return (NULL);
}