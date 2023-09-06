#ifndef HTTPSERVER_HTTPRESPONSE_H
#define HTTPSERVER_HTTPRESPONSE_H

#include "HttpRequest.h"
#include "ConnectionPool.h"
#include <map>

#include <sys/stat.h>
#include <mysql.h>


class HttpResponse {
public:

    enum class HttpCode {
        k200Ok = 200,
        k300MovedPermanently = 301,
        k400BadRequest = 400,
        k403Forbidden = 403,
        k404NotFound = 404,
        k500InternalServerError = 500,
    };

    HttpResponse();
    void initResponse(ConnectionPool* connPool, char* docRoot);
    void reset();
    void doResponse(HttpRequest& req, Buffer& inBuf, Buffer& outBuf);
    void addResponse(Buffer& buf);

    // getter & setter
    void setHttpCode(HttpCode httpCode);
    HttpCode getHttpCode() const;
    void setIsCloseConn(bool isClose);
    bool getIsCloseConn() const;
    const char* getRealFile() const;
    const struct stat& getFileStat() const;
    std::string getHttpCodeString() const;


private:
    void addResponseLine_(Buffer& buf);
    void addResponseHeader_(Buffer& buf);
    void addBlankLine_(Buffer& buf);
    void addContent_(Buffer& buf);

    

    // 用于设置响应行
    std::string version_;
    HttpCode httpCode_;

    // 用于设置响应头
    std::map<std::string, std::string> headers_;
    bool isCloseConn_;  // 是否保持连接

    // 用于设置响应体
    static const char kError400[];
    static const char kError403[];
    static const char kError404[];
    static const char kError500[];


    static const int kFilePathMaxLen = 200;
    static const int kFileNameMaxLen = 100;
    static char* docRoot_;
    struct stat fileStat_;
    char realFile_[kFilePathMaxLen];
    char url_[kFileNameMaxLen];

    MYSQL* mysql_;
    static ConnectionPool* connPool_;

};

#endif  // HTTPSERVER_HTTPRESPONSE_H