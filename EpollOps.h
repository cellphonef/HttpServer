#ifndef HTTPSERVER_EPOLLOPS_H
#define HTTPSERVER_EPOLLOPS_H

#include <sys/epoll.h>

int createEpollOrDie();
void addFd(int epollFd, int sockFd, bool oneShot=false, bool enableET=false);
void modFd(int epollFd, int sockFd, uint32_t events);
void delFd(int epollFd, int sockFd);







#endif