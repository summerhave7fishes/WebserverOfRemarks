#pragma once
// 确保头文件只包含一次
#include <sys/epoll.h>
#include <unordered_map>
#include <memory>
#include <unistd.h>
#include <pthread.h>
#include <map>
#include "Channel.h"
#include "Timer.h"
#include "Util.h"
class EventLoop;
class TimerNode;
class Channel;

// 整个处理连接的过程：包括解析头、解析请求体、分析URL、解析完成
enum ProcessState
{ // 服务器正在解析请求的 URI（统一资源标识符）。这包括解析请求的路径、查询参数和其他与 URI 相关的信息。
    STATE_PARSE_URI = 1,
    // 在这个阶段，服务器正在解析 HTTP 请求中的头部信息。这些头部信息包括诸如请求方法、HTTP 版本、各种头部字段等。
    STATE_PARSE_HEADERS,
    // 这个阶段可能表示服务器正在接收 HTTP 请求的主体（Body）部分。在一些情况下，特别是在处理 POST 请求时，服务器需要接收请求的主体数据。
    STATE_RECV_BODY,
    // 服务器可能正在分析已经解析的 URI、头部和主体信息。这可能包括对请求的验证、权限检查以及其他处理逻辑的执行。
    STATE_ANALYSIS,
    // 在这个阶段，表示整个 HTTP 请求处理过程已经完成。服务器可能已经成功响应了请求，并且可以关闭与客户端的连接或者继续处理下一个请求
    STATE_FINISH
};

enum URIState
{ // 表示需要重新解析 URI。这可能发生在初始解析时遇到了某些条件，要求再次尝试解析 URI，可能是因为 URI 的格式不完全符合标准或解析逻辑未能完全处理所有的情况。
    PARSE_URI_AGAIN = 1,
    // 表示 URI 解析过程中出现了错误。这可能是因为 URI 的格式错误、无效的字符、非法的编码或其他不符合 URI 标准的情况导致的解析错误。
    PARSE_URI_ERROR,
    // 表示 URI 解析成功。在这种情况下，服务器成功解析了客户端请求中的 URI，并且可以根据解析结果继续处理请求。
    PARSE_URI_SUCCESS
};

// 解析头的时候可能出现的状态
enum HeaderState
{
    PARSE_HEADER_SUCCESS = 1,
    PARSE_HEADER_AGAIN,
    PARSE_HEADER_ERROR
};

enum AnalysisState
{
    ANALYSIS_SUCCESS = 1,
    ANALYSIS_ERROR
};

enum ParseState
{
    H_START = 0,          // 开始解析阶段
    H_KEY,                // 解析到 HTTP 头部的键
    H_COLON,              // 解析到冒号
    H_SPACES_AFTER_COLON, // 冒号后面的空格
    H_VALUE,              // 解析到 HTTP 头部的值
    H_CR,                 // 回车符
    H_LF,                 // 换行符
    H_END_CR,             // 结束的回车符
    H_END_FL,             // 结束的换行符
};

enum ConnectionState
{
    // 表示连接已建立并处于活动状态。在这种情况下，通信双方可以进行数据交换和通信。
    H_CONNECTED = 0,
    // 表示连接正在断开过程中。在某些情况下，例如服务器或客户端正在关闭连接，但仍有一些尚未完成的操作需要处理，此时连接状态可以被设置为正在断开中。
    H_DISCONNECTING,
    // 表示连接已断开。在这种情况下，通信双方已经断开连接，不再交换数据。这可能是由于连接超时、网络故障或其他原因导致的连接断开。
    H_DISCONNECTED
};
enum HttpMethod
{
    METHOD_POST = 1,
    METHOD_GET,
    METHOD_HEAD
};
enum HttpVersion
{
    HTTP_10 = 1,
    HTTP_11
};
/*MIME是一种用于标识文件内容类型的标准
 * 类型对于网络通信和文件传输非常重要，因为它可以告诉接收端如何正确地解释和处理收到的数据*/
class MimeType
{
private:
    static void init();
    static std::unordered_map<std::string, std::string> mime;
    MimeType();
    MimeType(const MimeType &m);

public:
    static std::string getMime(const std::string &suffix);

private:
    /*这是一个私有的静态成员变量，它可能用于多线程环境下确保 init()
     * 函数只被执行一次的控制变量。pthread_once_t 是 POSIX
     * 线程库中的类型，通常用于执行一次性初始化。*/
    static pthread_once_t once_control;
};

class HttpData : public std::enable_shared_from_this<HttpData>
{
public:
    HttpData(EventLoop *loop, int connfd);
    ~HttpData() { close(fd_); }
    void reset();         // 把对象恢复到初始状态，以便可以处理新的HTTP请求，并且处理了定时器的清理
    void seperateTimer(); // 清理与定时器相关的请求数据或资源
    void linkTimer(std::shared_ptr<TimerNode> mtimer)
    {
        timer_ = mtimer;
    }

    // 返回一个事件
    std::shared_ptr<Channel> getChannel() { return channel_; }
    // 返回一个事件循环
    EventLoop *getLoop() { return loop_; }
    // 处理HTTP连接关闭时的一些清理工作，确保在关闭连接时不会出现任何悬挂指针或资源泄漏。
    void handleClose();
    // 创建一个新的HTTP链接
    void newEvent();

private:
    int fd_;
    std::weak_ptr<TimerNode> timer_;
    std::shared_ptr<Channel> channel_;
    EventLoop *loop_;
    std::shared_ptr<Channel> channel_;
    int fd_;
    std::string inBuffer_;  // inBuffer储存从客户端接受的HTTP协议，存在内存
    std::string outBuffer_; // outBuffer储存要发送给客户端的数据，存在内存
    bool error_;
    ConnectionState connectionState_;

    HttpMethod method_;
    HttpVersion HTTPVersion_;
    std::string fileName_;
    std::string path_;
    int nowReadPos_;
    ProcessState state_;
    ParseState hState_;
    bool keepAlive_;
    // 存解析的内容
    std::map<std::string, std::string> headers_;

    void handleRead();
    void handleWrite();
    void handleConn();
    ;
    void handleError(int fd, int err_num, std::string short_msg);
    URIState parseURI();
    HeaderState parseHeaders();
    AnalysisState analysisRequest();
};