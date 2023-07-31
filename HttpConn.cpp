#include "HttpConn.h"
#include "EpollOps.h"
#include <iostream>
#include <algorithm>
#include <map>
#include <sys/mman.h>

#include <fcntl.h>
#include <unistd.h>
#include <sys/uio.h>
#include <string.h>
#include <mysql/mysql.h>

using namespace std;


int HttpConn::epollFd = -1;


void HttpConn::initResponse(ConnectionPool* connPool, char* docRoot) {
   httpResponse_.initResponse(connPool, docRoot);
}

void HttpConn::init(int fd, struct sockaddr_in addr) {
    cout << "fd=" << fd_ << " HttpConn do init" << endl;
    fd_ = fd;
    addr_ = addr;
    reset();
}

void HttpConn::reset() {
    cout << "fd=" << fd_ << " reset" << endl;
    bytesToSend_ = 0;
    bytesHaveSend_ = 0;
    inputBuf_.retrieveAll();  // buffer是否应该shrink
    outputBuf_.retrieveAll();
    if (fileAddress_) {
        munmap(fileAddress_, httpResponse_.getFileStat().st_size);
        fileAddress_ = NULL;
    }
    HttpRequest dummy;
    httpRequest_.reset(dummy);
    httpResponse_.reset();
}

void HttpConn::process() {
    // 业务线程执行
    cout << "fd=" << fd_ << " start to parse" << endl;
    if (!httpRequest_.parse(inputBuf_)) {  // 解析失败
        cout << "parse failed!" << endl;
        outputBuf_.append("HTTP/1.1 400 Bad Request\r\n", 26);
        outputBuf_.append("\r\n", 2);
        outputBuf_.append("Your request has bad syntax or is inherently impossible to staisfy.\n", 68);
        iv_[0].iov_base = outputBuf_.beginRead();
        iv_[0].iov_len = outputBuf_.readableBytes();
        ivCount_ = 1;
        bytesToSend_ = iv_[0].iov_len;
    }


    if (httpRequest_.gotAll()) {  // 解析完成
        doRequest();
    } 

    modFd(epollFd, fd_, EPOLLOUT);  // 注册该fd的写事件
}

void HttpConn::doRequest() {
    cout << "fd=" << fd_ << " start to doRequest" << endl;
    httpResponse_.doResponse(httpRequest_, inputBuf_, outputBuf_);  // 根据request做出响应，响应状态反映了响应情况
    if(httpResponse_.getHttpCode() == HttpResponse::HttpCode::k200Ok) {
        cout << "fd=" << fd_ << " response ok" << endl;
        int fd = open(httpResponse_.getRealFile(), O_RDONLY);
        fileAddress_ = static_cast<char*>(mmap(0, httpResponse_.getFileStat().st_size, PROT_READ, MAP_PRIVATE, fd, 0));
        close(fd);
        iv_[0].iov_base = outputBuf_.beginRead();
        iv_[0].iov_len = outputBuf_.readableBytes();
        iv_[1].iov_base = fileAddress_;
        iv_[1].iov_len = httpResponse_.getFileStat().st_size;
        ivCount_ = 2;
        bytesToSend_ = iv_[0].iov_len + iv_[1].iov_len;
    } else {
        iv_[0].iov_base = outputBuf_.beginRead();
        iv_[0].iov_len = outputBuf_.readableBytes();
        ivCount_ = 1;
        bytesToSend_ = iv_[0].iov_len;
    }
    cout << "fd=" << fd_ << " bytesToSend=" << bytesToSend_ << endl;
}

void HttpConn::closeConn() {
    cout << "fd=" << fd_ << " HttpConn::closeConn()" << endl;
    delFd(epollFd, fd_);
    close(fd_);
    cout << "fd=" << fd_ << " HttpConn::closeConn() done!" << endl;
}


void HttpConn::recvMsg() { 
    inputBuf_.readFd(fd_);    
}   

bool HttpConn::sendMsg() {
    cout << "fd=" << fd_ << " sendMsg" << endl;
    // 主线程执行

    while (1) {  // 未发送完，考虑大文件
        int temp = writev(fd_, iv_, ivCount_);
        cout << "fd=" << fd_ << " temp=" << temp << endl;

        if (temp < 0) {  
            if (errno == EAGAIN) {  // 发送缓冲区满了，监听可写
                cout << "sendMsg error EAGAIN" << endl;
                modFd(epollFd, fd_, EPOLLOUT);
                return false;
            }
            // 其他错误
            if (fileAddress_) {
                munmap(fileAddress_, httpResponse_.getFileStat().st_size);
                fileAddress_ = NULL;
            }
            cout << "sendMsg error other error!" << endl;
            return true;
        }

        bytesToSend_ -= temp;
        bytesHaveSend_ += temp;
        cout << "fd=" << fd_ << " bytesToSend=" << bytesToSend_ << endl;
        cout << "fd=" << fd_ << " byteHaveSend=" << bytesHaveSend_ << endl; 
        
        if (bytesHaveSend_ > iv_[0].iov_len) {
            iv_[1].iov_base = fileAddress_ + (bytesHaveSend_ - iv_[0].iov_len);
            iv_[0].iov_len = 0;
            iv_[1].iov_len = bytesToSend_;
        } else {
            iv_[0].iov_base = outputBuf_.beginRead() + bytesHaveSend_;
            iv_[0].iov_len = bytesToSend_ - bytesHaveSend_;
        }   

        if (bytesToSend_ <= 0) {
            if (httpResponse_.getIsCloseConn()) {  // 短连接
                return true;
            } else {  // 长连接
                reset();
                modFd(epollFd, fd_, EPOLLIN);
                return false;
            }
        }
    }
}

int HttpConn::getFd() {
    return fd_;
}

void HttpConn::setFd(int fd) {
    fd_ = fd;
}

int HttpConn::getBytesToSend() const {
    return bytesToSend_;
}
