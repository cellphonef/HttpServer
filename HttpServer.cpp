#include "HttpServer.h"
#include "SocketOps.h"
#include "EpollOps.h"
#include "ErrorHandler.h"
#include "Timer.h"
#include "Logger.h"

#include <iostream>
#include <sys/epoll.h>
#include <sys/types.h>
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
    char* ret = getcwd(server_path, 200); (void)ret;
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
    // cout << "listenFd=" << listenFd_ << endl;
    // cout << "epollFd=" << epollFd_ << endl;
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
        LOG_INFO("remaining %d(s) TIMEOUT!", timeOutMs == -1 ? -1 : timeOutMs/1000);
        int numReadys = epoll_wait(epollFd_, activeFds_, kMaxEventNum, timeOutMs);
        LOG_INFO("%d events arrived", numReadys);
        if (numReadys == -1 && errno != EINTR) {  // 不可恢复错误
            LOG_ERROR("epoll_wait error");
        } else {
            for (int i = 0; i < numReadys; i++) {
                int fd = activeFds_[i].data.fd;
                uint32_t events = activeFds_[i].events;

                if (fd == listenFd_) {  // 监听描述符就绪
                    LOG_INFO("fd = %d readable", fd);
                    dealWithListen_();
                } else {  // 连接描述符就绪
                    if (events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR)) {  // 错误
                        dealWithError_(&userConns_[fd], events);
                    } else if (events & EPOLLOUT) {  // 可写
                        LOG_INFO("fd = %d writable", fd);
                        dealWithWrite_(&userConns_[fd]);
                    } else if (events & EPOLLIN) {  // 可读
                        LOG_INFO("fd = %d readable", fd);
                        dealWithRead_(&userConns_[fd]);
                    }
                }
            }
        }
    }
}

void HttpServer::dealWithListen_() {
    struct sockaddr_in clientAddr;
    bzero(&clientAddr, sizeof(clientAddr));
    int connFd = acceptOrDie(listenFd_, &clientAddr);
    if (connFd > kMaxFd || connFd < 0) {
        return;
    }
    userConns_[connFd].init(connFd, clientAddr);
    setNonBlocking(connFd);
    addFd(epollFd_, connFd, true);
    timerHeap_.addTimer(connFd, kTimerSlot, std::bind(&HttpServer::closeConn_, this, &userConns_[connFd]));
    LOG_INFO("connFd = %d connected!", connFd);
}

void HttpServer::dealWithWrite_(HttpConn* conn) {
    // 写逻辑：
    // 首先写数据，写完之后判断是否关闭连接，如果关闭则利用timer来关闭
    // 如果不关闭，则调整时间
    bool isClose = conn->sendMsg();
    LOG_INFO("fd = %d dealWithWrite done! KeepAlive = %d", conn->getFd(), isClose ? 0 : 1);
    if (isClose) {
        timerHeap_.delTimer(conn->getFd());
        return;
    }
    extentTime(conn);
}

void HttpServer::dealWithRead_(HttpConn* conn) {
    // 读逻辑：
    // 首先读数据，读完之后提交到线程池处理
    ssize_t n = conn->recvMsg();
    LOG_INFO("fd = %d dealWithRead done! received bytes = %d", conn->getFd(), (int)n);
    // if (n == 0) {
    //     timerHeap_.delTimer(conn->getFd());  // 可能线程还没来得及运行？
    //     return;
    // }
    extentTime(conn);
    LOG_INFO("fd = %d submit to thread pool!", conn->getFd());
    threadPool_.submit(conn);

}

void HttpServer::dealWithError_(HttpConn* conn, uint32_t events){
    LOG_INFO("fd = %d dealWithError", conn->getFd());
    timerHeap_.delTimer(conn->getFd()); 
}

void HttpServer::closeConn_(HttpConn* conn) {
    LOG_INFO("fd = %d is being close!", conn->getFd());
    conn->closeConn();
}

void HttpServer::extentTime(HttpConn* conn) {
    timerHeap_.adjustTimer(conn->getFd());
}

