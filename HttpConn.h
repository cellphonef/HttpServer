#ifndef HTTPSERVER_HTTPCONN_H
#define HTTPSERVER_HTTPCONN_H

#include "Buffer.h"
#include "HttpRequest.h"
#include "HttpResponse.h"
#include "ConnectionPool.h"
#include <netinet/in.h>


class HttpConn {
public:
    void init(int fd, struct sockaddr_in addr);
    void reset();

    bool sendMsg();
    void recvMsg();

    void process();
    void doRequest();
    void closeConn();

    int getFd();
    void setFd(int fd);
    int getBytesToSend() const;

    void initResponse(ConnectionPool* connPool, char* docRoot);
    static int epollFd;

private:  
    int fd_;
    struct sockaddr_in addr_;
    Buffer inputBuf_;
    Buffer outputBuf_;

    // 协议解析相关
    HttpRequest httpRequest_;
    HttpResponse httpResponse_;

    // 文件相关
    char* fileAddress_;
    struct iovec iv_[2];
    int ivCount_;
    size_t bytesToSend_;
    size_t bytesHaveSend_;
};




#endif // HTTPSERVER_HTTPCONN_H
