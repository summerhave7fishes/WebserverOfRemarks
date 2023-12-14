#include "AsyncLogging.h"

AsyncLogging::AsyncLogging(std::string logFileName_, int flushInterval) : flushInterval_(flushInterval), running_(false), basename_(logFileName_), thread_(std::bind(&AsyncLogging::threadFunc, this), "Logging"), mutex_(), cond_(mutex_), currentBuffer_(new Buffer), nextBuffer_(new Buffer), buffers_(), latch_(1)
{
    assert(logFileName_.size() > 1);
    currentBuffer_->bzero(); // 缓冲区清空为0
    nextBuffer_->bzero();
    buffers_.reserve(16); // 16个buffer
}

void AsyncLogging::append(const char *logline, int len)
{
    MutexLockGuard lock(mutex_);
    if (currentBuffer_->avail() > len)
    {
        currentBuffer_->append(logline, len);
    }
    // 如果没办法放进缓冲区,则放进vector中
    else
    {
        buffers_.push_back(currentBuffer_);
        currentBuffer_.reset(); // 重置指针的值
        if (nextBuffer_)
        {
            currentBuffer_ = std::move(nextBuffer_);
        }
        // 没有下一个buffer了
        else
        {
            currentBuffer_.reset(new Buffer);
            currentBuffer_->append(logline, len);
            cond_.notify();
        }
    }
}

void AsyncLogging::threadFunc()
{
    assert(running_ == true); // 如果running_==true会触发断言
    latch_.countDown();
    LogFile output(basename_); // 输出文件
    BufferPtr newBuffer1(new Buffer);
    BufferPtr newBuffer2(new Buffer);
    newBuffer1->bzero();
    newBuffer2->bzero();
    // 去写入磁盘的buffervector
    BufferVector buffersToWrite;
    buffersToWrite.resize(16);
    // 如果线程一直funning，则一直循环buffer写入到vector，并且从vector里循环写入basename_
    while (running_ == true)
    {
        // 条件为真，不会触发任何操作,这里去判断是否newBuffer存在，且里面没有数据，如果是true不发生任何事
        assert(newBuffer1 && newBuffer1->length() == 0);
        assert(newBuffer2 && newBuffer2->length() == 0);
        // 如果buffersToWrite为空，则交换buffers_
        assert(buffersToWrite.empty());
        {
            
            
            MutexLockGuard lock(mutex_);
            if (buffers_.empty())
            {
                // 如果vector is空，唤醒buffer可以去生产
                cond_.waitForSeconds(flushInterval_);
            }
            buffers_.push_back(currentBuffer_);
            currentBuffer_.reset();
            /*交换 buffersToWrite 和 buffers_ 的内容。这样做可以将 buffers_ 中的数据交给 buffersToWrite，同时将 buffers_ 清空，以便在之后的操作中填充新的数据。这样可以保证在交换后的 buffersToWrite 中包含了最新的要写入磁盘的缓冲区集合。*/
            buffersToWrite.swap(buffers_);
            if (!nextBuffer_)
            {
                nextBuffer_ = std::move(newBuffer2);
            }
        }
        assert(!buffersToWrite.empty());

        if (buffersToWrite.size() > 25)
        {
            /*它将删除从索引2到结尾的元素。这有助于限制数据量的增长，以避免内存消耗过大。*/
            buffersToWrite.erase(buffersToWrite.begin() + 2, buffersToWrite.end());
        }
        // 写进文件里面
        for (size_t i = 0; i < buffersToWrite.size(); ++i)
        {
            output.append(buffersToWrite[i]->data(), buffersToWrite[i]->length());
        }

        if (buffersToWrite.size() > 2)
        {
            /*
        这段注释的意思是舍弃未使用过的缓冲区，以避免内存被滥用。这是为了确保只有有效的缓冲区被写入到磁盘中，以防止内存被不必要的数据所占用，从而提高系统的效率。*/
            buffersToWrite.resize(2);
        }
        /*有两个缓冲区空间用于从前端写入后端。这两个缓冲区被命名为newBuffer1和newBuffer2。在处理过程中，这两个缓冲区会被交替使用，以确保日志记录的连续性。如果当前的newBuffer1或newBuffer2为空，则会从buffersToWrite中取出一个缓冲区来继续写入日志数据。这样可以确保始终有缓冲区用于写入日志数据，从而避免日志数据丢失的问题*/
        if (!newBuffer1)
        {
            assert(!buffersToWrite.empty());
            newBuffer1 = buffersToWrite.back();
            buffersToWrite.pop_back();
            newBuffer1->reset();
        }
        if (!newBuffer2)
        {
            assert(!buffersToWrite.empty());
            newBuffer2 = buffersToWrite.back();
            buffersToWrite.pop_back();
            newBuffer2->reset();
        }
        buffersToWrite.clear();
        output.flush();
    }
    output.flush();
}