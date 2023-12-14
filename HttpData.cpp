
#include "HttpData.h"
#include <iostream>
#include "Logging.h"
#include "EventLoop.h"
using namespace std;

pthread_once_t MimeType::once_control = PTHREAD_ONCE_INIT;
std::unordered_map<std::string, std::string> MimeType::mime;
/*如果设置了 EPOLLONESHOT 标志，那么该文件描述符在触发一次事件后就会被自动从 epoll 实例中移除，
这意味着需要再次将文件描述符添加到 epoll 实例中，如果需要继续监听它的事件。*/
const __uint32_t DEFAULT_EVENT = EPOLLIN | EPOLLET | EPOLLONESHOT;
const int DEFAULT_EXPIRED_TIME = 2000; // ms
const int DEFAULT_KEEP_ALIVE_TIME = 5 * 60 * 1000;
// 网站图标的数据
char favicon[555] = {
    '\x89',
    'P',
    'N',
    'G',
    '\xD',
    '\xA',
    '\x1A',
    '\xA',
    '\x0',
    '\x0',
    '\x0',
    '\xD',
    'I',
    'H',
    'D',
    'R',
    '\x0',
    '\x0',
    '\x0',
    '\x10',
    '\x0',
    '\x0',
    '\x0',
    '\x10',
    '\x8',
    '\x6',
    '\x0',
    '\x0',
    '\x0',
    '\x1F',
    '\xF3',
    '\xFF',
    'a',
    '\x0',
    '\x0',
    '\x0',
    '\x19',
    't',
    'E',
    'X',
    't',
    'S',
    'o',
    'f',
    't',
    'w',
    'a',
    'r',
    'e',
    '\x0',
    'A',
    'd',
    'o',
    'b',
    'e',
    '\x20',
    'I',
    'm',
    'a',
    'g',
    'e',
    'R',
    'e',
    'a',
    'd',
    'y',
    'q',
    '\xC9',
    'e',
    '\x3C',
    '\x0',
    '\x0',
    '\x1',
    '\xCD',
    'I',
    'D',
    'A',
    'T',
    'x',
    '\xDA',
    '\x94',
    '\x93',
    '9',
    'H',
    '\x3',
    'A',
    '\x14',
    '\x86',
    '\xFF',
    '\x5D',
    'b',
    '\xA7',
    '\x4',
    'R',
    '\xC4',
    'm',
    '\x22',
    '\x1E',
    '\xA0',
    'F',
    '\x24',
    '\x8',
    '\x16',
    '\x16',
    'v',
    '\xA',
    '6',
    '\xBA',
    'J',
    '\x9A',
    '\x80',
    '\x8',
    'A',
    '\xB4',
    'q',
    '\x85',
    'X',
    '\x89',
    'G',
    '\xB0',
    'I',
    '\xA9',
    'Q',
    '\x24',
    '\xCD',
    '\xA6',
    '\x8',
    '\xA4',
    'H',
    'c',
    '\x91',
    'B',
    '\xB',
    '\xAF',
    'V',
    '\xC1',
    'F',
    '\xB4',
    '\x15',
    '\xCF',
    '\x22',
    'X',
    '\x98',
    '\xB',
    'T',
    'H',
    '\x8A',
    'd',
    '\x93',
    '\x8D',
    '\xFB',
    'F',
    'g',
    '\xC9',
    '\x1A',
    '\x14',
    '\x7D',
    '\xF0',
    'f',
    'v',
    'f',
    '\xDF',
    '\x7C',
    '\xEF',
    '\xE7',
    'g',
    'F',
    '\xA8',
    '\xD5',
    'j',
    'H',
    '\x24',
    '\x12',
    '\x2A',
    '\x0',
    '\x5',
    '\xBF',
    'G',
    '\xD4',
    '\xEF',
    '\xF7',
    '\x2F',
    '6',
    '\xEC',
    '\x12',
    '\x20',
    '\x1E',
    '\x8F',
    '\xD7',
    '\xAA',
    '\xD5',
    '\xEA',
    '\xAF',
    'I',
    '5',
    'F',
    '\xAA',
    'T',
    '\x5F',
    '\x9F',
    '\x22',
    'A',
    '\x2A',
    '\x95',
    '\xA',
    '\x83',
    '\xE5',
    'r',
    '9',
    'd',
    '\xB3',
    'Y',
    '\x96',
    '\x99',
    'L',
    '\x6',
    '\xE9',
    't',
    '\x9A',
    '\x25',
    '\x85',
    '\x2C',
    '\xCB',
    'T',
    '\xA7',
    '\xC4',
    'b',
    '1',
    '\xB5',
    '\x5E',
    '\x0',
    '\x3',
    'h',
    '\x9A',
    '\xC6',
    '\x16',
    '\x82',
    '\x20',
    'X',
    'R',
    '\x14',
    'E',
    '6',
    'S',
    '\x94',
    '\xCB',
    'e',
    'x',
    '\xBD',
    '\x5E',
    '\xAA',
    'U',
    'T',
    '\x23',
    'L',
    '\xC0',
    '\xE0',
    '\xE2',
    '\xC1',
    '\x8F',
    '\x0',
    '\x9E',
    '\xBC',
    '\x9',
    'A',
    '\x7C',
    '\x3E',
    '\x1F',
    '\x83',
    'D',
    '\x22',
    '\x11',
    '\xD5',
    'T',
    '\x40',
    '\x3F',
    '8',
    '\x80',
    'w',
    '\xE5',
    '3',
    '\x7',
    '\xB8',
    '\x5C',
    '\x2E',
    'H',
    '\x92',
    '\x4',
    '\x87',
    '\xC3',
    '\x81',
    '\x40',
    '\x20',
    '\x40',
    'g',
    '\x98',
    '\xE9',
    '6',
    '\x1A',
    '\xA6',
    'g',
    '\x15',
    '\x4',
    '\xE3',
    '\xD7',
    '\xC8',
    '\xBD',
    '\x15',
    '\xE1',
    'i',
    '\xB7',
    'C',
    '\xAB',
    '\xEA',
    'x',
    '\x2F',
    'j',
    'X',
    '\x92',
    '\xBB',
    '\x18',
    '\x20',
    '\x9F',
    '\xCF',
    '3',
    '\xC3',
    '\xB8',
    '\xE9',
    'N',
    '\xA7',
    '\xD3',
    'l',
    'J',
    '\x0',
    'i',
    '6',
    '\x7C',
    '\x8E',
    '\xE1',
    '\xFE',
    'V',
    '\x84',
    '\xE7',
    '\x3C',
    '\x9F',
    'r',
    '\x2B',
    '\x3A',
    'B',
    '\x7B',
    '7',
    'f',
    'w',
    '\xAE',
    '\x8E',
    '\xE',
    '\xF3',
    '\xBD',
    'R',
    '\xA9',
    'd',
    '\x2',
    'B',
    '\xAF',
    '\x85',
    '2',
    'f',
    'F',
    '\xBA',
    '\xC',
    '\xD9',
    '\x9F',
    '\x1D',
    '\x9A',
    'l',
    '\x22',
    '\xE6',
    '\xC7',
    '\x3A',
    '\x2C',
    '\x80',
    '\xEF',
    '\xC1',
    '\x15',
    '\x90',
    '\x7',
    '\x93',
    '\xA2',
    '\x28',
    '\xA0',
    'S',
    'j',
    '\xB1',
    '\xB8',
    '\xDF',
    '\x29',
    '5',
    'C',
    '\xE',
    '\x3F',
    'X',
    '\xFC',
    '\x98',
    '\xDA',
    'y',
    'j',
    'P',
    '\x40',
    '\x0',
    '\x87',
    '\xAE',
    '\x1B',
    '\x17',
    'B',
    '\xB4',
    '\x3A',
    '\x3F',
    '\xBE',
    'y',
    '\xC7',
    '\xA',
    '\x26',
    '\xB6',
    '\xEE',
    '\xD9',
    '\x9A',
    '\x60',
    '\x14',
    '\x93',
    '\xDB',
    '\x8F',
    '\xD',
    '\xA',
    '\x2E',
    '\xE9',
    '\x23',
    '\x95',
    '\x29',
    'X',
    '\x0',
    '\x27',
    '\xEB',
    'n',
    'V',
    'p',
    '\xBC',
    '\xD6',
    '\xCB',
    '\xD6',
    'G',
    '\xAB',
    '\x3D',
    'l',
    '\x7D',
    '\xB8',
    '\xD2',
    '\xDD',
    '\xA0',
    '\x60',
    '\x83',
    '\xBA',
    '\xEF',
    '\x5F',
    '\xA4',
    '\xEA',
    '\xCC',
    '\x2',
    'N',
    '\xAE',
    '\x5E',
    'p',
    '\x1A',
    '\xEC',
    '\xB3',
    '\x40',
    '9',
    '\xAC',
    '\xFE',
    '\xF2',
    '\x91',
    '\x89',
    'g',
    '\x91',
    '\x85',
    '\x21',
    '\xA8',
    '\x87',
    '\xB7',
    'X',
    '\x7E',
    '\x7E',
    '\x85',
    '\xBB',
    '\xCD',
    'N',
    'N',
    'b',
    't',
    '\x40',
    '\xFA',
    '\x93',
    '\x89',
    '\xEC',
    '\x1E',
    '\xEC',
    '\x86',
    '\x2',
    'H',
    '\x26',
    '\x93',
    '\xD0',
    'u',
    '\x1D',
    '\x7F',
    '\x9',
    '2',
    '\x95',
    '\xBF',
    '\x1F',
    '\xDB',
    '\xD7',
    'c',
    '\x8A',
    '\x1A',
    '\xF7',
    '\x5C',
    '\xC1',
    '\xFF',
    '\x22',
    'J',
    '\xC3',
    '\x87',
    '\x0',
    '\x3',
    '\x0',
    'K',
    '\xBB',
    '\xF8',
    '\xD6',
    '\x2A',
    'v',
    '\x98',
    'I',
    '\x0',
    '\x0',
    '\x0',
    '\x0',
    'I',
    'E',
    'N',
    'D',
    '\xAE',
    'B',
    '\x60',
    '\x82',
};

void MimeType::init()
{
    // 什么样子表示什么协议
    mime[".html"] = "text/html";
    mime[".avi"] = "video/x-msvideo";
    mime[".bmp"] = "image/bmp";
    mime[".c"] = "text/plain";
    mime[".doc"] = "application/msword";
    mime[".gif"] = "image/gif";
    mime[".gz"] = "application/x-gzip";
    mime[".htm"] = "text/html";
    mime[".ico"] = "image/x-icon";
    mime[".jpg"] = "image/jpeg";
    mime[".png"] = "image/png";
    mime[".txt"] = "text/plain";
    mime[".mp3"] = "audio/mp3";
    mime["default"] = "text/html";
}

std::string MimeType::getMime(const std::string &suffix)
{ /*pthread_once 是一个用于执行一次性初始化的 POSIX 线程库函数。它接受两个参数：第一个参数是类型为 pthread_once_t 的控制变量，
 第二个参数是一个函数指针，指向需要执行的初始化函数。
 1.从h文件中的类可以看出，这个MimeType是一个单例模式（构造函数是private），此时用pthread_once函数可以保证MimeType类只初始化一次*/
    pthread_once(&once_control, MimeType::init);
    if (mime.find(suffix) == mime.end())
    {
        // 没在map中找到合适的，返回默认
        return mime["default"];
    }
    else
    {
        return mime[suffix];
    }
}
/*这里是构造函数的初始化，这里是将channel_里面的读事件、写事件、连接事件（Chenn_不进行具体的处理，只负责监听是否有这些事件发生，如果有则调用HttpData里面的具体事件处理函数）
在这种设计中，Channel 类负责监视文件描述符上的事件，包括读事件、写事件和连接事件。当这些事件发生时，Channel 类会调用已经绑定的特定处理函数。

而 HttpData 类中的 handleRead、handleWrite 和 handleConn 等事件处理函数会被 Channel 类调用，以实际处理这些事件。HttpData 类负责具体的业务逻辑处理，例如处理接收到的数据、准备要发送的数据以及处理连接的建立与关闭等。

这种设计模式将事件处理和业务逻辑处理分离开来，使代码结构更清晰，并且提高了代码的可维护性和可扩展性。*/
HttpData::HttpData(EventLoop *loop, int connfd) : loop_(loop), channel_(new Channel(loop, connfd)), fd_(connfd), error_(false), connectionState_(H_CONNECTED), method_(METHOD_GET), HTTPVersion_(HTTP_11), nowReadPos_(0), state_(STATE_PARSE_URI), hState_(H_START), keepAlive_(false)
{
    // 将chennl里面的事件与其具体的事件绑定起来
    channel_->setReadHandler(bind(&HttpData::handleRead, this));
    channel_->setWriteHandler(bind(&HttpData::handleWrite, this));
    channel_->setConnHandler(bind(&HttpData::handleConn, this));
}

/*重置HttpData主要做了以下几个方面：
1.文件名、路径清空
2.现在阅读的地方为0
3.整个处理连接过程置为STATE_PRARSE_URP
4.解析Http报文状态置为H_START
5.将解析的内容清空
*/
void HttpData::reset()
{
    fileName_.clear();
    path_.clear();
    nowReadPos_ = 0;
    state_ = STATE_PARSE_URI;
    hState_ = H_START;
    headers_.clear();
    /*lock()通常用于获取对某个对象的独占访问权限，以防止其他线程或进程同时对该对象进行修改。*/
    if (timer_.lock())
    {
        /*1.获得这个时间结点
         */
        shared_ptr<TimerNode> my_timer(timer_.lock());
        /*这个函数定义在Timer.h里面，有两个作用：清空该TimerNode绑定的HttpData，在优先级队列里面删掉这个TimerNode，等待到期时间，调用handleExpiredEvent()删掉delete_=true的结点*/
        my_timer->clearReq();
        // 将该TimeNode指针释放
        timer_.reset();
    }
}

// 将HttpData与TimerNode分离，清空该TimerNode绑定的HttpData，在优先级队列里面删掉这个TimerNode，等待到期时间，调用handleExpiredEvent()删掉delete_=true的结点
void HttpData::seperateTimer()
{
    if (timer_.lock())
    {
        shared_ptr<TimerNode> my_timer(timer_.lock());
        my_timer->clearReq();
        timer_.reset();
    }
}

/*
1.返回事件events：也许是可读，也许是可写？
2.读取inbuffer里面查看是否有数据
3.如果连接状态是正在解除连接：直接清空inbuffer
4.如果读取的数据小于0，则直接break
5.如果读取的数据==0，则将连接状态设置为解除连接状态，然后break
6.解析URI if (state_ == STATE_PARSE_URI)，解析URI， URIState flag = this->parseURI();
7.解析头 if (state_ == STATE_PARSE_HEADERS)，解析头HeaderState flag = this->parseHeaders();
8.如果是POST:得到content-length，然后设置为 state_ = STATE_ANALYSIS;解析数据AnalysisState flag = this->analysisRequest();
9.如果不是POST，则 state_ = STATE_ANALYSIS;状态改为STATE_FINISH
10.
10.以上通过while循环解析一次。

11.解析完成如果没有出错且outBuffer大于0， handleWrite();写入outBuffer中
 11.1 outBuffer储存要发送给客户的数据
12.如果没有出错并且inBufer里面还有数据，则继续递归调用handleRead()
13.如果当前连接没有发生错误且连接状态不是已断开，并且在事件监视中，需要将 EPOLLIN 这个可读事件标志加入到 events_ 变量中，以便在有数据可以读取时进行处理。
*/

void HttpData::handleRead()
{
    // 返回events_
    __uint32_t &events_ = channel_->getEvents();
    do
    {
        bool zero = false;
        /*zero是一个标志位，相当于是判断是否读取为0了*/
        int read_num = readn(fd_, inBuffer_, zero);
        LOG << "Request:" << inBuffer_;
        /*连接失败*/
        if (connectionState_ == H_DISCONNECTING)
        {
            inBuffer_.clear();
            break;
        }
        /*没有读取到数据*/
        if (read_num < 0)
        {
            perror("1");
            error_ = true;
            handleError(fd_, 400, "Bad Request");
            break;
        }
        /*zero是一个标志位，相当于是判断是否读取为0了*/
        else if (zero)
        { // 有请求出现但是读不到数据，可能是Request
            // Aborted，或者来自网络的数据没有达到等原因
            // 最可能是对端已经关闭了，统一按照对端已经关闭处理
            // error_ = true;
            connectionState_ = H_DISCONNECTING;
            if (read_num == 0)
            {
                break;
            }
        }
        /*1.STATE_PARSE_URI
        2.STATE_PARSE_HEADERS
        3.如果是post则：STATE_RECV_BODY,先解析长度，然后进入STATE_ANALYSIS
        4.如果不是post直接解析：STATE_ANALYSIS
        */
        if (state_ == STATE_PARSE_URI)
        {
            URIState flag = this->parseURI();
            // 一开始解析就遇到了问题，需要重新解析
            if (flag == PARSE_URI_AGAIN)
            {
                break;
            }
            // 如果遇到了错误
            else if (flag == PARSE_URI_ERROR)
            {
                perror("2");
                LOG << "FD =" << fd_ << "," << inBuffer_ << "******";
                inBuffer_.clear();
                error_ = true;
                handleError(fd_, 400, "Bad Request");
                break;
            }
            // 没有错误就进入解析头阶段
            else
            {
                state_ = STATE_PARSE_HEADERS;
            }
        }

        if (state_ == STATE_PARSE_HEADERS)
        { // 先解析Headers
            HeaderState flag = this->parseHeaders();
            if (flag == PARSE_HEADER_AGAIN)
            {
                break;
            }
            else if (flag == PARSE_HEADER_ERROR)
            {
                perror("3");
                error_ = true;
                handleError(fd_, 400, "Bad Request");
                break;
            }
            if (method_ == METHOD_POST)
            { // POST方法准备
                state_ = STATE_RECV_BODY;
            }
            else
            {
                /*如果请求方法不是 POST，那么将 state_ 设置为 STATE_ANALYSIS，这意味着正在分析 HTTP 请求的其他部分，而不需要接收请求体。*/
                state_ = STATE_ANALYSIS;
            }
        }

        // 如果是请求体
        if (state_ == STATE_RECV_BODY)
        {
            int content_length = -1;
            if (headers_.find("Content-length") != headers_.end())
            {
                content_length = stoi(headers_["Content-length"]);
            }
            else
            {
                error_ = true;
                handleError(fd_, 400, "Bad Request:Lack of argument(Content-length)");
                break;
            }
            if (static_cast<int>(inBuffer_.size()) < content_length)
            {
                break;
            }
            state_ = STATE_ANALYSIS;
        }

        // 进入分析数据状态
        if (state_ == STATE_ANALYSIS)
        {
            AnalysisState flag = this->analysisRequest();
            if (flag == ANALYSIS_SUCCESS)
            {
                state_ = STATE_FINISH;
                break;
            }
            else
            {
                error_ = true;
                break;
            }
        }

    } while (false); // 至少执行一次

    if (!error_)
    {
        // 此前第一轮解析已经完毕，如果输出缓冲区是空的，则写入到输出缓冲区
        if (outBuffer_.size() > 0)
        {
            // 写入outBuffer_
            handleWrite();
        }
        // 第一轮解析结束，如果输入缓冲区还有数据，继续解析如果输入缓冲区还有数据则继续读取
        if (!error_ && state_ == STATE_FINISH)
        {
            this->reset();
            if (inBuffer_.size() > 0)
            {
                if (connectionState_ != H_DISCONNECTING)
                    handleRead();
            }
        }
        /*如果当前连接没有发生错误且连接状态不是已断开，并且在事件监视中，需要将 EPOLLIN 这个可读事件标志加入到 events_ 变量中，以便在有数据可以读取时进行处理。*/
        else if (!error_ && connectionState_ != H_DISCONNECTED)
        {
            // 可读。告诉 epoll 实例，你对文件描述符的可读事件感兴趣，即便当前数据已经处理，你也希望在有新数据可读取时得到通知。
            /*events_ |= EPOLLIN; 是告诉 epoll 实例，你对文件描述符的可读事件感兴趣，希望在内核有数据可读时得到通知。这是一个从内核到应用程序的通知方向，告知你可以从内核中读取数据了。*/
            events_ |= EPOLLIN;
        }
    }
}

/*
该方法：将响应数据写入到连接中
1.outBuffer储存要发送给客户端的数据
2.如果outBuffer中有需要发送的数据，则尝试将数据写入到连接中，如果写入成功，则将 events_ 设置为 EPOLLOUT，以便在文件描述符（套接字）可写时得到通知，以继续写入数据。
告知 epoll 实例当前套接字可以写入数据，从而触发可写事件，以确保响应数据能够及时发送给客户端。
对于套接字，当它缓冲区有足够空间可以写入数据时，就会触发可写事件。但并不意味着数据已经被成功发送给了客户端。而是表示系统内核已经准备好接受发送数据的请求，能够进行写入操作了。
*/
void HttpData::handleWrite()
{
    if (!error_ && connectionState_ != H_DISCONNECTED)
    {
        __uint32_t &events_ = channel_->getEvents();
        // 如果
        if (writen(fd_, outBuffer_) < 0)
        {
            perror("writen");
            events_ = 0;
            error_ = true;
        }
        if (outBuffer_.size() > 0)
        { // 可写事件
            /*events_ |= EPOLLOUT; 是告诉 epoll 实例，你对文件描述符的可写事件感兴趣，希望在内核可以写入数据到文件描述符时得到通知。这是一个从应用程序到内核的通知方向，表示你已经准备好发送数据给内核，希望内核能够从你这里读取数据发送给客户端。*/
            events_ |= EPOLLOUT;
        }
    }
}

void HttpData::handleConn()
{
    // 先清除定时器，将HttpData与TimerNode分离
    seperateTimer();
    __uint32_t &events_ = channel_->getEvents();
    /*
    1.如果没有错误并且连接状态为已连接 (connectionState_ == H_CONNECTED)，则进行事件监听的处理。
    2.如果 events_ 不为 0（表示存在事件需要处理），根据是否为长连接来设置超时时间：如果是长连接 (keepAlive_ 为真)，设置超时时间为默认的长连接保持时间 DEFAULT_KEEP_ALIVE_TIME；否则设置为默认过期时间 DEFAULT_EXPIRED_TIME。
    3.如果同时存在可读和可写事件 ((events_ & EPOLLIN) & (events_ & EPOLLOUT))，则将 events_ 设置为 0，并只关注可写事件 (events_ |= EPOLLOUT)，同时设置为边缘触发模式（EPOLLET），然后将更新后的事件状态注册到事件监听器中。
    4.如果 events_ 为 0 但同时满足 keepAlive_ 为真，则设置 events_ 为同时关注可读和可写事件 (EPOLLIN | EPOLLOUT)，并设置超时时间为默认的长连接保持时间 DEFAULT_KEEP_ALIVE_TIME，然后将更新后的事件状态注册到事件监听器中。
    5.如果 events_ 为 0 且不满足 keepAlive_ 为真，则同样设置 events_ 为同时关注可读和可写事件 (EPOLLIN | EPOLLOUT)，但将超时时间设置为默认长连接保持时间的一半 (DEFAULT_KEEP_ALIVE_TIME >> 1)，然后将更新后的事件状态注册到事件监听器中。*/
    if (!error_ && connectionState_ == H_CONNECTED)
    {
        /*1.如果有事件处理，先设置为默认超时
          2.如果是长连接，设置为长连接超时
          3.如果同时同时存在可读可写事件，将events_设置只关注可写，设置为边缘触发*/
        if (events_ != 0)
        {
            int timeout = DEFAULT_EXPIRED_TIME;
            if (keepAlive_)
                timeout = DEFAULT_KEEP_ALIVE_TIME;
            if ((events_ & EPOLLIN) & (events_ & EPOLLOUT))
            {
                events_ = __uint32_t(0);
                events_ |= EPOLLOUT;
            }
            events_ |= EPOLLET;
            loop_->updatePoller(channel_, timeout);
        }
        // 当不满足 events_ != 0 条件但同时满足 keepAlive_ 为真，设置为同时关注可读和可写事件，设置超时时间为默认的长连接保持时间
        else if (keepAlive_)
        {
            events_ |= (EPOLLIN | EPOLLOUT);
            int timeout = DEFAULT_KEEP_ALIVE_TIME;
            loop_->updatePoller(channel_, timeout);
        }
        // 当既不满足 events_ != 0 也不满足 keepAlive_ 为真时,设置为可读可写，并且设置超时时间为长连接时间的一半
        else
        {
            events_ |= (EPOLLIN | EPOLLOUT);
            int timeout = (DEFAULT_KEEP_ALIVE_TIME >> 1);
            loop_->updatePoller(channel_, timeout);
        }
    }
    /*当发现连接正在关闭，并且存在可写事件时，将 events_ 设置为包含可写事件 EPOLLOUT 和边缘触发模式 EPOLLET。*/
    else if (!error_ && connectionState_ == H_DISCONNECTING && (events_ & EPOLLOUT))
    {
        events_ = (EPOLLOUT | EPOLLET);
    }
    // 如果出现关闭等意外情况，进行handleClose排入loop中，handleClose里面把channel移除loop中
    else
    {
        loop_->runInLoop(bind(&HttpData::handleClose, shared_from_this()));
    }
}

/*
1.首先查看是否有完整的请求行GET /index.html HTTP/1.1\r\n
2.其次截断出请求行赋值给request_line
3.其次将请求行后面到末尾的最后信息截断，赋值给str
4.得到请求方式
5.得到filename_，如果没有默认为index.html
*/

/*GET /index.html HTTP/1.1\r\n*/
URIState HttpData::parseURI()
{
    string &str = inBuffer_;
    string cop = str;
    // 得到完整的请求行再开始解析请求
    size_t pos = str.find('\r', nowReadPos_);
    if (pos < 0)
    {
        return PARSE_URI_AGAIN;
    }
    // 去掉请求行所占的空间，节省空间
    string request_line = str.substr(0, pos);
    if (str.size() > pos + 1)
    {
        /*str.substr(pos+1) 会生成一个新的字符串，其内容是从 str 的位置 pos+1（即 \r 后的下一个位置）开始一直到 str 末尾的部分。*/
        str = str.substr(pos + 1);
    }
    else
    {
        str.clear();
    }
    int posGet = request_line.find("GET");
    int posPost = request_line.find("POST");
    int posHead = request_line.find("HEAD");

    if (posGet >= 0)
    {
        pos = posGet;
        method_ = METHOD_GET;
    }
    else if (posPost >= 0)
    {
        pos = posPost;
        method_ = METHOD_GET;
    }
    else if (posHead >= 0)
    {
        pos = posHead;
        method_ = METHOD_HEAD;
    }
    else
    {
        return PARSE_URI_ERROR;
    }

    // filename
    pos = request_line.find("/", pos);
    /*如果 pos 的值小于 0，这意味着在 request_line 中没有找到斜杠字符 "/"。在这种情况下，它将执行 fileName_="index.html"; */
    if (pos < 0)
    {
        fileName_ = "index.html";
        HTTPVersion_ = HTTP_11;
        return PARSE_URI_SUCCESS;
    }
    /*GET /index.html HTTP/1.1\r\n*/
    else
    {
        size_t _pos = request_line.find(' ', pos); //_pos=空格
        if (_pos < 0)
        {
            return PARSE_URI_ERROR;
        }
        else
        {
            if (_pos - pos > 1)
            {
                fileName_ = request_line.substr(pos + 1, _pos - pos - 1); // index.html?param1=value1&param2=value2
                /*如果有？：GET /index.html?param1=value1&param2=value2 HTTP/1.1\r\n
                 */
                size_t __pos = fileName_.find('?');
                if (__pos >= 0)
                {
                    fileName_ = fileName_.substr(0, __pos);
                }
            }
            else
            {
                fileName_ = "index.html";
            }
        }
        pos = _pos; // pos:从/->空格
    }

    pos = request_line.find("/", pos); // pos=HTTP后面的/
    if (pos < 0)
    {
        return PARSE_URI_ERROR;
    }
    else
    {
        string ver = request_line.substr(pos + 1, 3); // 版本，放在ver里面
        if (ver == "1.0")
            HTTPVersion_ = HTTP_10;
        else if (ver == "1.1")
        {
            HTTPVersion_ = HTTP_11;
        }
        else
        {
            return PARSE_URI_ERROR;
        }
    }
    return PARSE_URI_SUCCESS;
}
/*
GET /index.html HTTP/1.1\r\n
Host: www.example.com\r\n
Connection: keep-alive\r\n
User-Agent: Mozilla/5.0\r\n
Accept: text/html\r\n
\r\n
1.利用状态机解析HTTP头，for循环，i到某一个应该被记录的值的时候，就进行记录，比如key_start或者key_end
2.可以看到，case的{}里面是next的i，所以if里面的str[i]是下一个位置的i
*/
HeaderState HttpData::parseHeaders()
{
    string &str = inBuffer_;
    int key_start = -1, key_end = -1, value_start = -1, value_end = -1;
    int now_read_line_begin = 0;
    bool notFinish = true;
    size_t i = 0;
    for (; i < str.size() && notFinish; ++i)
    {
        switch (hState_)
        {
        case H_START:
        {
            if (str[i] == '\n' || str[i] == '\r')
                break;
            hState_ = H_KEY;
            key_start = i;
            now_read_line_begin = i;
            break;
        }
        case H_KEY:
        { /*Host: www.example.com\r\n*/
            if (str[i] = ':')
            {
                key_end = i; //:
                if (key_end - key_start <= 0)
                    return PARSE_HEADER_ERROR;
                hState_ = H_COLON;
            }
            else if (str[i] == '\n' || str[i] == '\r')
            {
                return PARSE_HEADER_ERROR;
            }
            break;
        }
        case H_COLON:
        { /*Host: www.example.com\r\n,i到空格*/
            if (str[i] = ' ')
            {
                hState_ = H_SPACES_AFTER_COLON;
            }
            else
            {
                return PARSE_HEADER_ERROR;
            }
            break;
        }
        /*Host: www.example.com\r\n*/
        case H_SPACES_AFTER_COLON:
        {
            hState_ = H_VALUE;
            value_start = i;
            break;
        }

        case H_VALUE:
        {
            /*Host: www.example.com\r\n*/
            if (str[i] == '\r')
            {
                hState_ = H_CR;
                value_end = i;
                if (value_end - value_start <= 0)
                    return PARSE_HEADER_ERROR;
            }
            else if (i - value_start > 255)
            {
                return PARSE_HEADER_ERROR;
            }
            break;
        }
        /*H_CR是回车符\r，下一个就是换行符，状态机到\n的时候，开始将key以及value整理出来放在headers_中*/
        case H_CR:
        {
            if (str[i] == '\n')
            {
                hState_ = H_LF;
                // 这里在上面的状态机中，已经在遍历到该遍历的位置将i赋值到key_start,以及key_end中了
                string key(str.begin() + key_start, str.begin() + key_end);
                string value(str.begin() + value_start, str.begin() + value_end);
                headers_[key] = value;
                now_read_line_begin = i;
            }
            else
            {
                return PARSE_HEADER_ERROR;
            }
            break;
        }
        /*H_LF是换行符\n，一般来说后面有两种状态:到达第二行，或者后面是\r，这意味着是截至行。

        在HTTP协议中，头部字段和消息体之间以及消息体内部的每行都使用回车换行符(\r\n)来进行分隔。而头部字段结束的标志是两个连续的回车换行符(\r\n\r\n)，表示头部字段的结束。这个空行标志着头部字段的结束和消息体的开始。*/
        case H_LF:
        {
            if (str[i] == '\r')
            {
                hState_ = H_END_CR;
            }
            // 表达到第二行，并非结束行
            else
            {
                key_start = i;
                hState_ = H_KEY;
            }
            break;
        }
        case H_END_CR:
        {
            // 结束行的\r
            if (str[i] == H_END_FL)
            {
                hState_ = H_END_FL;
            }
            else
            {
                return PARSE_HEADER_ERROR;
            }
            break;
        }
        // 结束行\n
        case H_END_FL:
        {
            notFinish = false;
            key_start = i;
            now_read_line_begin = i;
            break;
        }
        }
    }
    if (hState_ == H_END_FL)
    { /*str是头结尾（索引i） 开始到末尾的子字符串*/
        str = str.substr(i);
        return PARSE_HEADER_SUCCESS;
    }
    /*1.首先如果解析成功了，会在if语句将now_read_line_begin=i，并且返回解析成功，下面这两句就不会执行了
    2.如果没有解析成功，跳到这两步重新再去解析，此时now_read_line_begin=0,并且返回重新解析头
    */
    str = str.substr(now_read_line_begin);
    return PARSE_HEADER_AGAIN;
}
