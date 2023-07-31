#include "SocketOps.h"
#include "ErrorHandler.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/errno.h>


int createNonblockingOrDie() {

    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd == -1) {
        errSys("socket error");
    }

    // 常用代码片段，将fd设置为非阻塞
    // 先获取旧的标志集合再或上新的标志得到新的标志集合
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1) {
        errSys("fcntl F_GETFL error");
    }
    flags |= O_NONBLOCK;
    if (fcntl(fd, F_SETFL, flags) == -1) {
        errSys("fcntl F_SETFL error");
    }
    return fd;
}

void bindOrDie(int sockFd, int port) {
    /*
    * sockaddr_in IPv4套接字地址结构
    *
    * struct sockaddr_in {
    *     uint8_t sin_len;  // 结构长度，除非涉及路由套接字否则无需设置和检查它
    *     sa_family_t sin_family;  
    *     in_port_t sin_port;
    *     struct in_addr sin_addr;
    *     char sin_zero[8];
    * }
    */

    struct sockaddr_in servAddr;
    bzero(&servAddr, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servAddr.sin_port = htons(port);

    if (bind(sockFd, (sockaddr*)&servAddr, sizeof(servAddr)) == -1) {
        errSys("bind error");
    }

}

void listenOrDie(int sockFd) {
    if (listen(sockFd, 4096) == -1) {
        errSys("listen error");
    }
}

int acceptOrDie(int listenFd, struct sockaddr_in* clientAddr) {
    socklen_t clientLen = sizeof(*clientAddr);
    int connFd = accept(listenFd, (sockaddr*)(&clientAddr), &clientLen);  // FIXME: c++11 cast 

    if (connFd < 0) {
        if (errno != EAGAIN) {
            errSys("accept error");
        }
    }

    return connFd;

}

void setNonBlocking(int sockFd) {
    int flags = fcntl(sockFd, F_GETFL, 0);
    if (flags == -1) {
        errSys("fcntl F_GETFL error");
    }
    flags |= O_NONBLOCK;
    if (fcntl(sockFd, F_SETFL, flags) == -1) {
        errSys("fcntl F_SETFL error");
    }
}

