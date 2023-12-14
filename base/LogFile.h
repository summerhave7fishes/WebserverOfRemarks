#pragma once

#include"noncopyable.h"
#include<string.h>
#include<iostream>
#include <memory>
#include"MutexLock.h"
#include"FileUtil.h"
using namespace std;
class LogFile:noncopyable{
    public:
    LogFile(const std::string &basename,int flushEveryN=1024);
    ~LogFile();

    void append(const char *logline,int len);
    void flush();
    bool rollFile();

    private:
    void append_unlocked(const char* logline,int len);
    const std::string basename_;
    const int flushEveryN_;
    int count_;
    //根据mutex_对象给其加锁
    std::unique_ptr<MutexLock> mutex_;
    //根据file_对象向磁盘中写入信息
    std::unique_ptr<AppendFile> file_;

};
