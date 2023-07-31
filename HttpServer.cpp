#include "HttpServer.h"
#include "SocketOps.h"
#include "EpollOps.h"
#include "ErrorHandler.h"
#include "Timer.h"

#include <iostream>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <strings.h>
#include <string.h>
#include <unistd.h>

using namespace std;

HttpServer::HttpServer(int port,
                       int threadNum,
                       std::string userName, std::string passWd, 
                       std::string databaseName, int sqlNum) 
    : port_(port), threadPool_(threadNum), 
      userName_(userName), passWd_(passWd),
      databaseName_(databaseName), sqlNum_(sqlNum) {
    listenFd_ = createNonblockingOrDie();
    bindOrDie(listenFd_, port_);
    listenOrDie(listenFd_);
    setNonBlocking(listenFd_);
    userConns_ = new HttpConn[kMaxFd];
    activeFds_ = new epoll_event[kMaxEventNum];

    char server_path[200];
    getcwd(server_path, 200);
    char root[6] = "/root";
    docRoot_ = (char *)malloc(strlen(server_path) + strlen(root) + 1);
    strcpy(docRoot_, server_path);
    strcat(docRoot_, root);

    eventLoopInit_();
    threadPoolInit_();
    databaseInit_();
   
}

HttpServer::~HttpServer() {
    close(epollFd_);
    close(listenFd_);
    delete[] userConns_;
    delete[] activeFds_;
    delete connPool_;
    free(docRoot_);
}

void HttpServer::eventLoopInit_() {
    epollFd_ = createEpollOrDie();
    HttpConn::epollFd = epollFd_;
    addFd(epollFd_, listenFd_);
    cout << "listenFd=" << listenFd_ << endl;
    cout << "epollFd=" << epollFd_ << endl;
}

void HttpServer::threadPoolInit_() {
    threadPool_.start();
}

void HttpServer::databaseInit_() {
    connPool_ = ConnectionPool::getInstance();
    connPool_->init("127.0.0.1", userName_, passWd_, databaseName_, 3306, sqlNum_);
    userConns_->initResponse(connPool_, docRoot_);
}

void HttpServer::loop() {
    while (true) {
        int timeOutMs = timerHeap_.getNextTick(); 
        cout << "timeOutMs=" << timeOutMs << endl;
        int numReadys = epoll_wait(epollFd_, activeFds_, kMaxEventNum, timeOutMs);
        cout << "numReadys=" << numReadys << endl;
        if (numReadys == -1 && errno != EINTR) {  // 不可恢复错误
            errSys("epoll_wait error");
        } else {
            for (int i = 0; i < numReadys; i++) {
                int fd = activeFds_[i].data.fd;
                uint32_t events = activeFds_[i].events;

                if (fd == listenFd_) {  // 监听描述符就绪
                    cout << "listenFd = " << fd << " readable" << endl; 
                    dealWithListen_();
                } else {  // 连接描述符就绪
                    if (events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR)) {  // 错误
                        if (events & (EPOLLRDHUP | EPOLLHUP)) {
                            cout << "fd = " << fd << "EPOLLRDHUP or EPOLLHUP half close" << endl; 
                        }
                        dealWithError_(&userConns_[fd], events);
                    } else if (events & EPOLLOUT) {  // 可写
                        cout << "fd = " << fd << " writable" << endl; 
                        dealWithWrite_(&userConns_[fd]);
                    } else if (events & EPOLLIN) {  // 可读
                        cout << "fd = " << fd << " readable" << endl; 
                        dealWithRead_(&userConns_[fd]);
                    }
                }
            }
        }
    }
}

void HttpServer::dealWithListen_() {
    cout << "dealWithListen" << endl;
    struct sockaddr_in clientAddr;
    bzero(&clientAddr, sizeof(clientAddr));
    int connFd = acceptOrDie(listenFd_, &clientAddr);
    if (connFd > kMaxFd || connFd < 0) {
        return;
    }
    cout << "connFd=" << connFd << endl;
    userConns_[connFd].init(connFd, clientAddr);
    setNonBlocking(connFd);
    addFd(epollFd_, connFd);
    timerHeap_.addTimer(connFd, kTimerSlot, std::bind(&HttpServer::closeConn_, this, &userConns_[connFd]));
}

void HttpServer::dealWithWrite_(HttpConn* conn) {
    // 写逻辑：
    // 首先写数据，写完之后判断是否关闭连接，如果关闭则利用timer来关闭
    // 如果不关闭，则调整时间
    cout << "fd=" << conn->getFd() << " dealWithWrite" << endl;
    bool isClose = conn->sendMsg();
    cout << "fd=" << conn->getFd() << " dealWithWrite done! isClose=" << isClose << endl;
    if (isClose) {
        timerHeap_.delTimer(conn->getFd());
        return;
    }
    extentTime(conn);
}

void HttpServer::dealWithRead_(HttpConn* conn) {
    // 读逻辑：
    // 首先读数据，读完之后提交到线程池处理
    cout << "fd=" << conn->getFd() << " dealWithRead" << endl;
    conn->recvMsg();
    extentTime(conn);
    threadPool_.submit(conn);
}

void HttpServer::dealWithError_(HttpConn* conn, uint32_t events){
    if (events & EPOLLRDHUP) {
        shutdown(conn->getFd(), SHUT_RD);
    } else {
        timerHeap_.delTimer(conn->getFd());
    }
}

void HttpServer::closeConn_(HttpConn* conn) {
    cout << "fd=" << conn->getFd() << " closeConn" << endl;
    conn->closeConn();
}

void HttpServer::extentTime(HttpConn* conn) {
    timerHeap_.adjustTimer(conn->getFd());
}

