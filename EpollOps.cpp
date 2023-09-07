#include "EpollOps.h"
#include "ErrorHandler.h"
#include "Logger.h"

#include <iostream>

using namespace std;


int createEpollOrDie() {
    int epollfd = epoll_create(5);
    if (epollfd == -1) {
        errSys("epoll_create error");
    }
    return epollfd;
}

/*
* struct epoll_event {
*     __uint32_t events;  // epoll事件
*    epoll_data_t data;  // 用户数据  
* }
* 
* typedef union epoll_data {
*     void* ptr;    
*     int fd;  // 事件所属对应的fd
*     uint32_t u32;
*     uint64_t u64;
* } epoll_data_t;
*
*/

void addFd(int epollfd, int sockFd, bool oneShot, bool enableET) {
    struct epoll_event event;
    event.data.fd = sockFd;
    event.events = EPOLLIN | EPOLLRDHUP;
    if (enableET) {
        event.events |= EPOLLET;
    }
    if (oneShot) {
        event.events |= EPOLLONESHOT;
    }

    int ret = epoll_ctl(epollfd, EPOLL_CTL_ADD, sockFd, &event);
    if (ret < 0) {
        errSys("epoll_ctl EPOLL_CTL_ADD error");
    }
}

void delFd(int epollFd, int sockFd) {
    int ret = epoll_ctl(epollFd, EPOLL_CTL_DEL, sockFd, NULL);

    LOG_INFO("fd = %d delFd done!", sockFd);
    if (ret < 0) {
        LOG_ERROR("fd = %d EPOLL_CTL_DEL ERROR!", sockFd);
    }
}

void modFd(int epollFd, int sockFd, uint32_t events) {
    struct epoll_event event;
    event.data.fd = sockFd;
   
    event.events = events | EPOLLRDHUP | EPOLLONESHOT;
   
    

    int ret = epoll_ctl(epollFd, EPOLL_CTL_MOD, sockFd, &event);
    if (ret < 0) {
        LOG_ERROR("fd = %d EPOLL_CTL_MOD ERROR!", sockFd);
    } 
}
