#ifndef HTTPSERVER_SOCKETOPS_H
#define HTTPSERVER_SOCKETOPS_H

int createNonblockingOrDie();
int connect(int sockFd);
void bindOrDie(int sockFd, int port);
void listenOrDie(int sockFd);
int acceptOrDie(int listenFd, struct sockaddr_in* clientAddr);
void setNonBlocking(int sockFd);


#endif











