#include "EpollOps.h"
#include "ErrorHandler.h"

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

void addFd(int epollfd, int sockFd, bool enableET) {
    struct epoll_event event;
    event.data.fd = sockFd;
    event.events = EPOLLIN;
    if (enableET) {
        event.events |= EPOLLET;
    }

    int ret = epoll_ctl(epollfd, EPOLL_CTL_ADD, sockFd, &event);
    if (ret < 0) {
        errSys("epoll_ctl EPOLL_CTL_ADD error");
    }
    cout << "fd=" << sockFd << " addFd done!" << endl;
}

void delFd(int epollFd, int sockFd) {
    int ret = epoll_ctl(epollFd, EPOLL_CTL_DEL, sockFd, NULL);
    if (ret < 0) {
        cout << "del fd error! fd=" << sockFd << endl;
        errSys("epoll_ctl EPOLL_CTL_DEL error");
    }
    cout << "fd=" << sockFd <<" delFd done!" << endl;
}

void modFd(int epollFd, int sockFd, uint32_t events) {
    struct epoll_event event;
    event.data.fd = sockFd;
    event.events = events | EPOLLRDHUP | EPOLLONESHOT;

    int ret = epoll_ctl(epollFd, EPOLL_CTL_MOD, sockFd, &event);
    if (ret < 0) {
        cout << "mod fd error! fd=" << sockFd << endl;
        errSys("epoll_ctl EPOLL_CTL_MOD error");
    } 
    cout << "fd=" << sockFd << " modFd done!" << endl;
}
