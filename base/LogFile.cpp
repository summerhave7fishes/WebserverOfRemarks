#include "LogFile.h"
using namespace std;

LogFile::LogFile(const std::string &basename, int flushEveryN) : basename_(basename), flushEveryN_(flushEveryN), count_(0), mutex_(new MutexLock)
{
    /*file_ 是一个指向 AppendFile 对象的智能指针，通过 new 关键字进行动态分配，用于处理文件追加操作。在这里，reset 函数被用来重新指定 file_ 指针所指向的对象。basename是要打开的文件名字
    其中 LogFile 类负责处理日志文件，而 AppendFile 类负责打开文件并设置缓冲区，以便进行文件写入操作。*/
    file_.reset(new AppendFile(basename));
}

LogFile::~LogFile() {}

void LogFile::append(const char *logline, int len)
{
    /*在 LogFile::append 函数中加锁互斥量，确保在多线程环境中对日志文件的访问是线程安全的。在函数开始时锁定互斥量，在函数结束时自动解锁互斥量，可以确保在多线程环境中对共享资源（比如日志文件）的访问是线程安全的。这样可以避免多个线程同时写入日志文件而造成的数据不一致性和错误。*/
    MutexLockGuard lock(*mutex_);
    append_unlocked(logline, len);
} // lock 对象超出其作用域，即此处，其析构函数会被调用

void LogFile::flush()
{
    MutexLockGuard lock(*mutex_);
    file_->flush(); // 存入磁盘
}


/*当写入数量>=flushEveryN的时候，从缓冲区刷入文件*/
void LogFile::append_unlocked(const char *logline, int len)
{
    file_->append(logline, len);
    ++count_;
    if (count_ >= flushEveryN_)
    {
        count_ = 0;
        file_->flush();
    }
}
