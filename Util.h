#pragma once
#include <cstdlib>
#include <string>

/*它通常被用作系统调用（比如 read 和 write）的返回值类型，以便能够表示可能的错误情况或特殊值。*/
ssize_t readn(int fd, void *buff, size_t n);
ssize_t readn(int fd, std::string &inBuffer, bool &zero);
ssize_t readn(int fd, std::string &inBuffer);

ssize_t writen(int fd, void *buff, size_t n);
ssize_t writen(int fd, std::string &sbuff);

/*handle_for_sigpipe() 函数用于处理 SIGPIPE 信号。SIGPIPE 信号通常在向一个已关闭的 socket 写数据时会被触发，为了防止程序因未处理该信号而意外终止，通常会将其忽略。该函数中的 sigaction 结构被设置为忽略 SIGPIPE 信号。*/
void handle_for_sigpipe();
/*函数用于将给定的套接字文件描述符 fd 设置为非阻塞模式。*/
int setSocketNonBlocking(int fd);

void setSocketNodelay(int fd);

void setSocketNolinger(int fd);

void shutDownWR(int fd);

int socket_bind_listen(int port);