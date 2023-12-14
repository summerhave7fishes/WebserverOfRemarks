#include "Logging.h"
#include "AsyncLogging.h"
#include <time.h>
#include <sys/time.h>
static pthread_once_t once_control_ = PTHREAD_ONCE_INIT;
static AsyncLogging *AsyncLogger_;

std::string Logger::logFileName_ = "./myserver.log";

void once_init()
{
    AsyncLogger_ = new AsyncLogging(Logger::getLogFileName());
    // 直接开始线程
    AsyncLogger_->start();
}

void output(const char *msg, int len)
{
    pthread_once(&once_control_, once_init);
    AsyncLogger_->append(msg, len);
}

Logger::Impl::Impl(const char *fileName, int line) : stream_(), line_(line), basename_(fileName)
{
    // 把时间放进去
    formatTime();
}

void Logger::Impl::formatTime()
{
    struct timeval tv;
    time_t time;
    char str_t[26] = {0};
    gettimeofday(&tv, NULL);
    time = tv.tv_sec;
    struct tm *p_time = localtime(&time);
    strftime(str_t, 26, "%Y-%m-%d %H:%M:%S\n", p_time);
    stream_ << str_t;
}

Logger::Logger(const char *fileName, int line)
    : impl_(fileName, line)
{
}

Logger::~Logger()
{
    impl_.stream_ << " -- " << impl_.basename_ << ':' << impl_.line_ << '\n';
    const LogStream::Buffer &buf(stream().buffer());
    output(buf.data(), buf.length());
}