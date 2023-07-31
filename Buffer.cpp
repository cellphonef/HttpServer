#include "Buffer.h"

#include <iostream>
#include <algorithm>
#include <sys/uio.h>
#include <unistd.h>
#include <sys/types.h>


using namespace std;

const char Buffer::kCRLF[] = "\r\n";

Buffer::Buffer(size_t initialSize) 
    : buffer_(kCheapPrepend+initialSize),
      readIndex_(kCheapPrepend),
      writeIndex_(kCheapPrepend) {}


size_t Buffer::readableBytes() const {
    return writeIndex_ - readIndex_;
}

size_t Buffer::writableBytes() const {
    return buffer_.size() - writeIndex_;
}

size_t Buffer::prependableBytes() const {
    return readIndex_;
}

const char* Buffer::peek() const {
    return begin_() + readIndex_;
}

const char* Buffer::findCRLF() const {
    const char* crlf = std::search(peek(), beginWrite(), kCRLF, kCRLF+2);
    return crlf == beginWrite() ? NULL : crlf;
}

void Buffer::retrieve(size_t len) {
    if (len < readableBytes()) {
        readIndex_ += len;
    } else {
        retrieveAll();
    }
}

void Buffer::retrieveUtil(const char* end) {
    retrieve(end - peek());
}

void Buffer::retrieveAll() {
    readIndex_ = kCheapPrepend;
    writeIndex_ = kCheapPrepend;
}

void Buffer::append(const char* data, size_t len) {
    ensureWritableBytes(len);
    copy(data, data+len, beginWrite());
    writeIndex_ += len;
}

void Buffer::ensureWritableBytes(size_t len) {
    if (writableBytes() < len) {
        makeSpace_(len);
    }
}


    
int Buffer::readFd(int fd) {
    // saved an ioctl()/FIONREAD call to tell how much to read
    char extrabuf[65536];
    struct iovec vec[2];
    const size_t writable = writableBytes();
    vec[0].iov_base = begin_()+writeIndex_;
    vec[0].iov_len = writable;
    vec[1].iov_base = extrabuf;
    vec[1].iov_len = sizeof(extrabuf);
    // when there is enough space in this buffer, don't read into extrabuf.
    // when extrabuf is used, we read 128k-1 bytes at most.
    const int iovcnt = (writable < sizeof extrabuf) ? 2 : 1;
    const ssize_t n = readv(fd, vec, iovcnt);
    cout << "fd=" << fd <<" writable=" << writable << endl;
    cout << "fd=" << fd << " n=" << n << endl;
    if (n < 0) {
        // FIXME: 停止程序
        exit(1);
    }
    else if (static_cast<size_t>(n) <= writable)
    {   
        cout << "fd=" << fd << " writeIndex += n" << endl;
        writeIndex_ += n;
    }
    else
    {
        writeIndex_ = buffer_.size();
        append(extrabuf, n - writable);
    }
  
    return n;

}

char* Buffer::beginWrite() {
    return begin_() + writeIndex_;
}

const char* Buffer::beginWrite() const {
    return begin_() + writeIndex_;
}


char* Buffer::beginRead() {
    return begin_() + readIndex_;
}

const char* Buffer::beginRead() const {
    return begin_() + readIndex_;
}


char* Buffer::begin_() {
    return &*buffer_.begin();
}

const char* Buffer::begin_() const {
    return &*buffer_.begin();
}

void Buffer::makeSpace_(size_t len) {
    if (writableBytes() + prependableBytes() < len + kCheapPrepend) {
        buffer_.resize(writeIndex_+len);
        moveReadableToFront_();
    } else {
        moveReadableToFront_();
    }
}

void Buffer::moveReadableToFront_() {
    size_t readable = readableBytes();
    copy(begin_()+readIndex_, begin_()+writeIndex_, begin_()+kCheapPrepend);
    readIndex_ = kCheapPrepend;
    writeIndex_ = kCheapPrepend + readable;
}