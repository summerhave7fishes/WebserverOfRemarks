#pragma once
#include<string>
#include"noncopyable.h"

/*公开继承了 noncopyable 类。通过公开继承 noncopyable 类，AppendFile 类将继承 noncopyable 类的特性，即禁止拷贝和赋值操作。
在任何情况下都不会有可用的拷贝构造函数和赋值运算符重载函数，这样可以确保该类的对象不会被复制，从而避免由于意外的拷贝而导致的问题。*/

/*FileUtil是最底层的文件类，封装了Log文件的打开、写入并在类析构的时候关闭文件，底层使用了标准IO，该append函数直接向文件写。*/
class AppendFile:noncopyable{
    public:
    explicit AppendFile(std::string filename);
    ~AppendFile();

    void append(const char *logline,const size_t len);
    void flush();

    private:
    size_t write(const char *logline,size_t len);
    FILE *fp_;//fp_ 是一个指向 FILE 结构的指针，可以用来表示一个文件流。
    char buffer_[64*1024];

};