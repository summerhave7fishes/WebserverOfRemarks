#pragma once
#include <assert.h>
#include <string.h>

#include <string>

#include "noncopyable.h"
class AsyncLogging;
const int kSmallBuffer = 4000;
const int kLargeBuffer = 4000 * 1000;

/*日志流类，用于处理不同类型数据的转换并将其存储到缓冲区中。它重载了多种数据类型的输出操作符 << 以便将数据转换成字符串并存储到缓冲区中。*/
/*表示定义一个模板类或函数*/
template <int SIZE>
class FixedBuffer : noncopyable
{
public:
    FixedBuffer() : cur_(data_) {}
    ~FixedBuffer() {}

    void append(const char *buf, size_t len)
    { /*所以，if(avail() > static_cast<int>(len)) 意味着如果可用的空间大于要复制的数据长度 len，则执行下面的操作。在这种情况下，memcpy 被用来将 buf 中的数据复制到 cur_，然后 cur_ 指针被移动到已经复制的数据之后，以便在下一次写入时不会覆盖已复制的数据*/
        if (avail() > static_cast<int>(len))
        {
            memcpy(cur_, buf, len);
            cur_ += len;
        }
    }

    // 返回现在的数据
    const char *data() const { return data_; }

    // 返回现在的长度cur_ 是指向缓冲区中当前位置的指针，而 data_ 是指向缓冲区起始位置的指针。
    int length() const { return static_cast<int>(cur_ - data_); }
    // 返回现在指针指向的缓存的位置
    char *current() { return cur_; }
    // 可用大小
    int avail() const { return static_cast<int> end() - cur_; }
    // 从现在的位置增加len个
    void add(size_t len) { cur_ += len; }

    void reset() { cur_ = data_; }
    void bzero() { memset(data_, 0, sizeof data_); }

private:
    const char *end() const { return data_ + sizeof data_; }
    char data_[SIZE];
    char *cur_;
};

class LogStream : noncopyable
{
public:
    typedef FixedBuffer<kSmallBuffer> Buffer;

    LogStream &operator<<(bool v)
    {
        buffer_.append(v ? "1" : "0", 1);
        return *this;
    }
    /*将short重载为字符，同时添加到内部的缓冲区Buffer*/
    LogStream &operator<<(short);
    LogStream &operator<<(unsigned short);
    LogStream &operator<<(int);
    LogStream &operator<<(unsigned int);
    LogStream &operator<<(long);
    LogStream &operator<<(unsigned long);
    LogStream &operator<<(long long);
    LogStream &operator<<(unsigned long long);
    LogStream &operator<<(const void *);

    // float转换为double类型
    LogStream &operator<<(float v)
    {
        *this << static_cast<double>(v);
        return *this;
    }
    LogStream &operator<<(double);
    LogStream &operator<<(long double);

    // 将v添加到buffer中
    LogStream &operator<<(char v)
    {
        buffer_.append(&v, 1);
        return *this;
    }

    LogStream &operator<<(const char *str)
    {
        if (str)
            buffer_.append(str, strlen(str));
        else
            buffer_.append("(null)", 6);
        return *this;
    }

    LogStream &operator<<(const unsigned char *str)
    {
        return operator<<(reinterpret_cast<const char *>(str));
    }

    LogStream &operator<<(const std::string &v)
    {
        buffer_.append(v.c_str(), v.size());
        return *this;
    }

    void append(const char *data, int len) { buffer_.append(data, len); }
    const Buffer &buffer() const { return buffer_; }
    void resetBuffer() { buffer_.reset(); }

private:
    void staticCheck();
    template <typename T>
    void formatInteger(T);
    Buffer buffer_;

    static const int kMaxNumericSize = 32;
};
