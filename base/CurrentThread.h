#pragma once
#include <stdint.h>
namespace CurrentThread
{
    /*每个线程都会维护一份独立的变量副本，而不是像全局变量那样所有线程共享同一份全局实体。这意味着每个线程都可以独立地读取和修改这个变量，而不会影响其他线程中的同名变量。*/
    extern __thread int t_cachedTid;      // 相当于每一个线程都有一个独立的t_cachedTid副本
    extern __thread char t_tidString[32]; // 储存线程ID字符串

    extern __thread int t_tidStringLength;    // 储存线程ID长度
    extern __thread const char *t_threadName; // 声明了一个指向常量字符的指针，用于指向线程名称的字符数组。

    /*将线程ID缓存在t_tidString中，长度放进t_tidStringLength中*/
    void cacheTid();
    /*是为了获取当前线程的线程ID和线程ID的字符串表示形式，并将它们缓存起来，以便其他部分的代码可以方便地访问它们，例如用于日志记录或其他目的。
    函数内部的 __builtin_expect 和 0
    的比较是一种优化提示，它告诉编译器线程的线程ID通常不会等于0，因此条件成立的可能性较低。这有助于编译器进行优化，提高代码的执行效率。
    将线程ID缓存在t_tidString中，长度放进t_tidStringLength中同时返回t_cachedTid*/
    inline int tid()
    {
        if (__builtin_expect(t_cachedTid == 0, 0))
        {
            cacheTid();
        }
        return t_cachedTid;
    }

    inline const char *tidString()
    {
        return t_tidString;
    }

    inline int tidStringLength()
    {
        return t_tidStringLength;
    }

    inline const char *name() { return t_threadName; }
}