#include "Server.h"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <functional>
#include "Util.h"
#include "base/Logging.h"
/*
1.创建一个事件循环线程池eventLoopThreadPool_（作用是传入主事件循环线程，通过threadNum开启时间循环事件线程）
2.创建一个acceptChannel，与主事件循环loop_绑定起来，用于监听
3.创建一个listenFd，与监听port_,套接字上进行监听和处理连接请求
4.将listenFd与acceptChannel_绑定
*/
Server::Server(EventLoop *loop, int threadNum, int port) : loop_(loop), threadNum_(threadNum), eventLoopThreadPool_(new EventLoopThreadPool(loop_, threadNum)), started_(false), acceptChannel_(new Channel(loop_)), port_(port), listenFd_(socket_bind_listen(port_))
{
    acceptChannel_->setFd(listenFd_);
    handle_for_sigpipe();
    /*设置为非阻塞模式*/
    if (setSocketNonBlocking(listenFd_) < 0)
    {
        perror("set socket no block failed");
        abort();
    }
}

void Server::start()
{
    /*eventLoopThreadPool_事件循环线程池开始，创建numThreads个事件循环线程，将线程放入threads_数组，事件放进事件循环数组loops_*/
    eventLoopThreadPool_->start();
    acceptChannel_->setEvents(EPOLLIN | EPOLLET);
    /*1.创建一个处理事件的fd
    2.从事件循环线程池eventLoopThreadPool_中取出一个事件循环
    3.

    */
    acceptChannel_->setReadHandler(bind(&Server::handNewConn, this));
    /*把acceptChannel_绑定的主事件循环loop_添加到poller上*/
    acceptChannel_->setConnHandler(bind(&Server::handThisConn, this));
    // 将监听Channel加入poller
    loop_->addToPoller(acceptChannel_, 0);
    started_ = true;
}

void Server::handNewConn()
{
    struct sockaddr_in client_addr;
    memset(&client_addr, 0, sizeof(struct sockaddr_in));
    socklen_t client_addr_len = sizeof(client_addr);
    int accept_fd = 0;
    while ((accept_fd = accept(listenFd_, (struct sockaddr *)&client_addr, &client_addr_len)) > 0)
    {
        EventLoop *loop = eventLoopThreadPool_->getNextLoop();
        LOG << "New connection from" << inet_ntoa(client_addr.sin_addr) << ":" << ntohs(client_addr.sin_port);

        // 限制服务器的最大并发连接数
        if (accept_fd >= MAXFDS)
        {
            close(accept_fd);
            continue;
        }
        // 设置为非阻塞
        if (setSocketNonBlocking(accept_fd) < 0)
        {
            LOG << "Set non block failed!";
            return;
        }
        /*禁用 Nagle 算法，提高网络传输的实时性。*/
        setSocketNodelay(accept_fd);
        /*核心代码：将现在的事件循环和accept_fd绑定*/
        shared_ptr<HttpData> req_info(new HttpData(loop,accept_fd));
        //在HttpData的构造函数里面去new Channel
        req_info->getChannel()->setHolder(req_info);
        /*
        1.将 HttpData::newEvent 方法与 req_info 对象绑定在一起，创建一个可调用的对象，然后将这个对象作为任务添加到事件循环中。
        2.将这个回调函数放进处理EventLoop的loop的私有成员变量pendingFunctors_数组中
        */
        loop->queueInLoop(std::bind(&HttpData::newEvent,req_info));
    }
    acceptChannel_->setEvents(EPOLLIN|EPOLLET);
}