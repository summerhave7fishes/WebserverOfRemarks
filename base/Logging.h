#pragma once
#include "LogStream.h"
#include <stdio.h>

class AsyncLogging;
class Logger
{
public:
    Logger(const char *filename, int line);
    ~Logger();
    LogStream &stream()
    {
        return impl_.stream_;
    }

    static void setLogFileName(std::string fileName)
    {
        logFileName_ = fileName;
    }

    static std::string getLogFileName() { return logFileName_; }

private:
    /*跟踪日志的来源以及帮助开发人员定位问题。*/
    class Impl
    {
    public:
        Impl(const char *fileName, int line);
        void formatTime();
        LogStream stream_;
        int line_;
        std::string basename_;
    };
    Impl impl_;
    static std::string logFileName_;
};
#define LOG Logger(__FILE__, __LINE__).stream()