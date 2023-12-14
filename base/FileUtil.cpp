#include "FileUtil.h"

using namespace std;
/*将 fp_ 初始化为通过 fopen 函数打开文件 filename 而得到的文件指针。fopen 函数用于以指定的模式打开文件，"ae" 模式表示以追加模式打开文件，如果文件不存在则创建该文件。*/
AppendFile::AppendFile(string filename) : fp_(fopen(filename.c_str(), "ae"))
{
    /*setbuffer 函数用于设置文件缓冲区，它设置了 fp_ 的缓冲区为 buffer_，缓冲区的大小为 sizeof buffer_。*/
    setbuffer(fp_, buffer_, sizeof buffer_);
}
AppendFile::~AppendFile()
{
    fclose(fp_);
}

/*write()函数是写入缓冲区，appen()函数是将len长度的写入缓冲区*/
void AppendFile::append(const char *logline, const size_t len)
{
    size_t n = this->write(logline, len);
    size_t remain=len-n;
    while(remain>0)
    {
        //logline + n 指向剩余数据的起始地址，remain 是剩余数据的长度。x 是成功写入的数据长度。
        size_t x=this->write(logline+n,remain);
        /*如果写入的长度为0说明发生了错误*/
        if(x==0)
        {
            /*检查 fp_ 的错误指示器，看看是否有错误发生。*/
            int err=ferror(fp_);
            if(err) fprintf(stderr,"AppendFile::append() failed!\n");
            break;
        }
        /*n+=x;：更新成功写入的数据长度*/
        n+=x;
        remain=len-n;
    }
}

/*这个函数调用了 fflush 函数，用于刷新缓冲区，确保所有的写入操作都已经被提交到对应的文件中。在这里，它会将文件缓冲区中的数据立即写入到磁盘中。*/
void AppendFile::flush(){
    fflush(fp_);
}


/*fwrite_unlocked 是一个不带锁的版本的 fwrite 函数。它用于将数据从指定的缓冲区 logline 写入到文件 fp_ 中。
logline 是要写入文件的数据的起始地址。
len 是要写入文件的数据的长度。
1 是要写入的每个数据单元的大小。在这里，1 表示每个字符是一个字节，因此整个字符串中的每个字符都会被写入文件中。
fp_ 是文件指针，表示要写入的目标文件。
函数的返回值是成功写入的数据单元数。如果函数成功写入了 len 个数据单元，则返回值应该等于 len。需要注意的是，这里的写操作是不带锁的，因此在多线程环境下使用时需要注意线程安全性。*/
size_t AppendFile::write(const char *logline, size_t len)
{
    return fwrite_unlocked(logline, 1, len, fp_);

}