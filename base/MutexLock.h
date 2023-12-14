#pragma once
#include <pthread.h>
#include <cstdio>
#include "noncopyable.h"

class MutexLock : noncopyable
{
public:
    MutexLock() { pthread_mutex_init(&mutex, NULL); }
    ~MutexLock()
    {
        //先锁定
        pthread_mutex_lock(&mutex);
        pthread_mutex_destroy(&mutex);
    }
    //加锁
    void lock(){pthread_mutex_lock(&mutex);}
    //解锁
    void unlock(){pthread_mutex_unlock(&mutex);}
    //获得锁
    /*& 是一个一元操作符，用于获取变量的地址。在这个例子中，get() 函数返回一个指向 mutex 互斥锁的指针。通过 return &mutex;，返回的是 mutex 互斥锁的地址，即指向 mutex 的指针。*/
    pthread_mutex_t *get(){return &mutex;}
private:
    pthread_mutex_t mutex;

    /*友元类，不受访问权限的影响*/
private:
    friend class Condition;
};

/*在这个类中，MutexLockGuard对象在构造函数中获取互斥锁，即调用mutex.lock()，而在析构函数中释放互斥锁，即调用mutex.unlock()。这保证了无论以何种方式离开作用域（正常离开、异常抛出等），都能保证互斥锁的正确释放。*/
class MutexLockGuard:noncopyable{
    public:
    explicit MutexLockGuard(MutexLock &_mutex):mutex(_mutex){mutex.lock();}
    ~MutexLockGuard(){mutex.unlock();}
    private:
    MutexLock &mutex;
};
