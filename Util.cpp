#include "Util.h"
#include <unistd.h>
#include <signal.h>
#include <string.h> // C 标准库中的头文件
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>

const int MAX_BUFF = 4096;

/*第一个函数使用一个指向缓冲区的指针,返回读取的总字节数，同时返回一个buff指针指向缓冲区*/
ssize_t readn(int fd, void *buff, size_t n)
{
    // 初始化为n，--，如果nleft=0则读取完毕
    size_t nleft = n;
    ssize_t nread = 0;
    ssize_t readSum = 0;

    // buff开头
    char *ptr = (char *)buff;
    while (nleft > 0)
    {
        // 从fd中读取nleft个字节，储存到ptr中
        if ((nread = read(fd, ptr, nleft)) < 0)
        { // EINTR“系统调用被中断”的错误码。
            if (errno == EINTR)
                nread = 0;
            else if (errno == EAGAIN)
            {
                // 代表已经读取完成
                return readSum;
            }
            else
            {
                return -1;
            }
        }
        else if (nread == 0)
            break;
        // nread>0的情况
        readSum += nread;
        nleft -= nread;
        ptr += nread;
    }
    return readSum;
}

// 一次性读取4096,zero是一个标志位，相当于是判断是否读取为0了，如果是，就直接break
/*返回一个std::string对象，大小*/
ssize_t readn(int fd, std::string &inBuffer, bool &zero)
{
    ssize_t nread = 0;
    ssize_t readSum = 0;
    /*无限循环结构*/
    while (true)
    {
        char buff[MAX_BUFF];
        if ((nread = read(fd, buff, MAX_BUFF)) < 0)
        {
            // 表示系统调用被中断，通常是由于收到了某个信号。在这种情况下，可以选择重新调用 read 函数，以确保数据的完整读取。
            if (errno == EINTR)
                continue;
            else if (errno == EAGAIN)
                return readSum;
            else
            {
                perror("read error");
                return -1;
            }
        }
        else if (nread == 0)
        {
            zero = true;
            break;
        }

        // 如果读取数据>0
        readSum += nread;
        inBuffer += std::string(buff, buff + nread);
    }
    return readSum;
}

ssize_t readn(int fd, std::string &inBuffer)
{
    ssize_t nread = 0;
    ssize_t readSum = 0;
    while (true)
    {
        char buff[MAX_BUFF];
        if ((nread = read(fd, buff, MAX_BUFF)) < 0)
        {
            if (errno == EINTR)
                continue;
            else if (errno == EAGAIN)
            {
                return readSum;
            }
            else
            {
                perror("read error");
                return -1;
            }
        }
        else if (nread == 0)
        {
            // printf("redsum = %d\n", readSum);
            break;
        }
        // printf("before inBuffer.size() = %d\n", inBuffer.size());
        // printf("nread = %d\n", nread);
        readSum += nread;
        // buff += nread;
        inBuffer += std::string(buff, buff + nread);
        // printf("after inBuffer.size() = %d\n", inBuffer.size());
    }
    return readSum;
}

ssize_t writen(int fd, void *buff, size_t n)
{
    // 还剩多少可以读取
    size_t nleft = n;
    // 读取一次写了多少
    ssize_t nwritten = 0;
    // 一共写了多少
    ssize_t writeSum = 0;
    char *ptr = (char *)buff;
    while (nleft > 0)
    {
        if ((nwritten = write(fd, ptr, nleft)) <= 0)
        {
            if (nwritten < 0)
            {
                if (errno == EINTR)
                {
                    nwritten = 0;
                    continue;
                    ;
                }
                else if (errno == EAGAIN)
                {
                    return writeSum;
                }
                else
                {
                    return -1;
                }
            }
        }
        writeSum += nwritten;
        nleft -= nwritten;
        ptr += nwritten;
    }
    return writeSum;
}

/*把sbuff写入fd
关于已经把sbuff转为了ptr为什么还要去判断nwrite是否==writeSum：
当 write 函数无法将所有数据一次性写入文件描述符时，只有部分数据被写入，此时 nwritten 将小于 nleft。这可能发生在文件描述符被设置为非阻塞模式时，当内核的发送缓冲区已满时会返回 EAGAIN 错误码，导致不能一次性将所有数据写入。因此，为了确保未写入的数据不会丢失，需要将未写入的部分保留在 sbuff 中，以便下次继续写入。*/
ssize_t writen(int fd, std::string &sbuff)
{
    size_t nleft = sbuff.size();
    ssize_t nwritten = 0;
    ssize_t writeSum = 0;
    /*将 sbuff 中的内容转换为一个 C 风格字符串，并将指向这个 C 风格字符串的指针赋值给了 ptr。*/
    const char *ptr = sbuff.c_str();
    while (nleft > 0)
    {
        /*从指针 ptr 指向的位置开始，向文件描述符 fd 所代表的文件中写入最多 nleft 个字节的数据。*/
        if ((nwritten == write(fd, ptr, nleft)) <= 0)
        {
            if (nwritten < 0)
            {
                if (errno == EINTR)
                {
                    nwritten = 0;
                    continue;
                }
                else if (errno == EAGAIN)
                    break;
                else
                    return -1;
            }
        }
        writeSum += nwritten;
        nleft -= nwritten;
        // 的作用是更新指针 ptr 的位置，使其指向尚未写入的数据的起始位置。这样在下一次循环中，write 函数就可以从新的位置开始写入数据。
        ptr += nwritten;
    }
    /*如果写入的总字节数等于 sbuff 字符串的长度，说明所有数据都已经被写入，此时将 sbuff 清空。*/
    if (writeSum == static_cast<int>(sbuff.size()))
    {
        sbuff.clear();
    }
    /*写入的总字节数小于 sbuff 的长度，说明还有部分数据没有被写入，因此使用 substr 函数截取剩余的部分赋值给 sbuff，以便下次继续写入。*/
    else
    {
        sbuff = sbuff.substr(writeSum);
    }
    return writeSum;
}

/*它的作用是为 SIGPIPE 信号注册一个处理函数。SIGPIPE 信号在进程向已关闭的管道写入数据时会产生。
这段代码的作用是忽略 SIGPIPE 信号的默认处理行为，确保当程序遇到管道破裂的情况时不会意外终止。*/
void handle_for_sigpipe()
{
    struct sigaction sa;
    /*void *memset(void *ptr, int value, size_t num);
将结构体 sa 所占内存空间的内容全部设置为零。*/
    memset(&sa, '\0', sizeof(sa));
    /*sa_handler 是 struct sigaction 结构体中的一个成员，用来指定信号处理函数。此处的意思是的作用是将信号 sa 的处理方式设置为忽略。这意味着当程序收到对应的信号时，将不会执行该信号的默认处理操作，而是直接忽略该信号。*/
    sa.sa_handler = SIG_IGN;
    /*将其设置为零表示在处理信号时不启用任何特殊的标志或选项。设置特定的处理标志或选项可能会改变对信号的处理方式，比如使用 SA_RESTART 标志可以使被信号中断的系统调用自动重启。但在这里将其设置为零，意味着使用操作系统的默认行为来处理这个信号。*/
    sa.sa_flags = 0;
    /*int sigaction(int signum, const struct sigaction *act, struct sigaction *oldact);
    SIGPIPE 是一个表示管道破裂的信号常量。
    NULL 则是一个空指针，表示不获取或不保存原有的信号处理信息。
    这行代码的意思是，如果对 SIGPIPE 信号的处理注册遇到问题，就直接返回，不做进一步的处理。通常这种情况下会发生在信号处理注册失败的情况下，可能是因为给定的信号处理函数或者标志不合法。
*/
    if (sigaction(SIGPIPE, &sa, NULL))
        return;
}
/*将文件描述符 fd 设置为非阻塞模式*/
int setSocketNonBlocking(int fd)
{
    /*这行代码的作用是获取文件描述符 fd 的状态标志，并将其存储在整型变量 flag 中。这些状态标志可以包括文件的访问模式和文件状态信息，比如文件是否设置为非阻塞模式等。*/
    int flag = fcntl(fd, F_GETFL, 0);
    if (flag == -1)
        return -1;
    flag |= O_NONBLOCK;
    /*使用 fcntl 函数来设置文件描述符 fd 的状态标志为 flag。如果设置失败，fcntl 函数将返回 -1。这样的判断通常用于检查是否成功修改了文件描述符的状态标志。*/
    if (fcntl(fd, F_SETFL, flag) == -1)
        return -1;
}
/*禁用 Nagle 算法，提高网络传输的实时性。*/
void setSocketNodelay(int fd)
{
    int enable = 1;
    setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (void *)&enable, sizeof(enable));
}

void setSocketNoLinger(int fd)
{
    struct linger linger_;
    linger_.l_onoff = 1;   // 启用SO_LINGER选项
    linger_.l_linger = 30; // 等待套接字关闭的最长时间为 30 秒
    setsockopt(fd, SOL_SOCKET, SO_LINGER, (const char *)&linger_, sizeof(linger_));
}
/*关闭套接字 fd 的写端。第二个参数是一个整型值，用于指定要关闭的方向。在这里，SHUT_WR 表示关闭套接字的写端，即禁止写入操作。*/
void shutDownWR(int fd)
{
    shutdown(fd, SHUT_WR);
    // printf("shutdown\n");
}
//
int socket_bind_listen(int port)
{
    if (port < 0 || port > 65535)
        return -1;

    // 创建socket(IPv4 + TCP)，返回监听描述符
    int listen_fd = 0;
    // 创建一个监听套接字
    if ((listen_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
        return -1;

    // 消除bind时“Address already in use"出错误,设置 SO_REUSEADDR 选项可以确保在关闭套接字后，可以立即重用该端口
    int optval = 1;
    if (setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) == -1)
    {
        close(listen_fd);
        return -1;
    }

    struct sockaddr_in server_addr;
    /*bzero 函数用于将 server_addr 所指向的地址中的内容全部设置为 0。*/
    bzero((char *)&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    /*一个服务器应该接受所有到达其 IP 地址的数据包。*/
    server_addr.sin_addr.s_addr - htonl(INADDR_ANY);
    server_addr.sin_port = htons((unsigned short)port);
    // 绑定
    if (bind(listen_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == 1)
    {
        close(listen_fd);
        return -1;
    }

    // 开始监听,2048 是指定套接字可以排队等待的最大连接数。当有新的连接请求到达时，如果连接队列已满，则新的连接请求会被拒绝。
    if (listen(listen_fd, 2048) == -1)
    {
        close(listen_fd);
        return -1;
    }
    // 无效监听描述符
    if (listen_fd == -1)
    {
        close(listen_fd);
        return -1;
    }
    return listen_fd;
}
