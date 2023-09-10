#ifndef HTTPSERVE_BUFFER_H
#define HTTPSERVE_BUFFER_H

#include <vector>
#include <stddef.h>
#include <sys/types.h>


class Buffer {
public:
    static const size_t kCheapPrepend = 8;
    static const size_t kInitialSize = 4096;

    explicit Buffer(size_t initialSize = kInitialSize);

    size_t readableBytes() const;
    size_t writableBytes() const;
    size_t prependableBytes() const;

    const char* peek() const;  // 窥视，只查看数据不取出数据

    void retrieve(size_t len);  // 从buffer中取走len长度的数据
    void retrieveAll();  // 取走所有数据
    void retrieveUtil(const char* end);

    void append(const char* data, size_t len);

    void ensureWritableBytes(size_t len);

    const char* findCRLF() const;  // 查找CRLF的位置


    char* beginWrite();
    const char* beginWrite() const;

    char* beginRead();
    const char* beginRead() const;
    
    ssize_t readFd(int fd);  
    int writeFd(int fd);


private:
    char* begin_();
    const char* begin_() const;

    static const char kCRLF[];

    void makeSpace_(size_t len);
    void moveReadableToFront_();

    std::vector<char> buffer_;
    size_t readIndex_;
    size_t writeIndex_;

};

#endif // HTTPSERVE_BUFFER_H