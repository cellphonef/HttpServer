#ifndef HTTPSERVER_HTTPSERVER_H
#define HTTPSERVER_HTTPSERVER_H

#include "ThreadPool.h"
#include "Timer.h"
#include "HttpConn.h"
#include "ConnectionPool.h"

#include <string>
#include <unordered_map>

/*
采用半同步半反应堆并发模式

*/
class HttpServer {
public:

    HttpServer(int port, 
               int threadNum,
               std::string userName, std::string passWd,
               std::string databaseName, int sqlNum);
    ~HttpServer();

    void loop();    


private:

    void threadPoolInit_();
    void eventLoopInit_();
    void databaseInit_();

    void dealWithRead_(HttpConn* conn);
    void dealWithWrite_(HttpConn* conn);
    void dealWithListen_();
    void dealWithError_(HttpConn* conn, uint32_t events);

    void closeConn_(HttpConn* conn);

    void extentTime(HttpConn* conn);


    // socket相关
    int port_;
    int listenFd_;
    static const int kMaxFd = 65536;
    
    // eventloop相关
    int epollFd_;
    static const int kMaxEventNum = 4096;
    struct epoll_event* activeFds_;
   
    // 定时相关
    TimerHeap timerHeap_;
    

    // 线程池相关
    ThreadPool<HttpConn> threadPool_;

    // 数据库相关
    ConnectionPool* connPool_;
    std::string userName_;
    std::string passWd_;
    std::string databaseName_;
    int sqlNum_;

    // http连接相关
    HttpConn* userConns_;

    // 根路径
    char* docRoot_;

    

};

#endif