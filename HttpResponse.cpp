
#include "HttpResponse.h"
#include "MutexLockGuard.h"
#include "Mutex.h"

#include <iostream>

#include <string>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <mysql/mysql.h>


using namespace std;

Mutex responseMtx;
map<string, string> users;

const char HttpResponse::kError400[] = "Your request has bad syntax or is inherently impossible to staisfy.\n";
const char HttpResponse::kError403[] = "You do not have permission to get file form this server.\n";
const char HttpResponse::kError404[] = "The requested file was not found on this server.\n";
const char HttpResponse::kError500[] = "There was an unusual problem serving the request file.\n";

ConnectionPool* HttpResponse::connPool_ = NULL;
char* HttpResponse::docRoot_ = NULL;

HttpResponse::HttpResponse() : isCloseConn_(true), mysql_(NULL) {}

void HttpResponse::initResponse(ConnectionPool* connPool, char* docRoot) {
    connPool_ = connPool;
    docRoot_ = docRoot;
    MYSQL* mysql = NULL;
    ConnectionPoolRAII(&mysql, connPool);
    if (mysql_query(mysql, "SELECT username, passwd FROM user")) {
        // FIXME: LOG
        exit(1);
    }
    MYSQL_RES* res = mysql_store_result(mysql);
    while (MYSQL_ROW row = mysql_fetch_row(res)) {
        string name(row[0]);
        string passWd(row[1]);
        users[name] = passWd;
    }
}
 
void HttpResponse::reset() {
    memset(realFile_, '\0', kFilePathMaxLen);
    memset(url_, '\0', kFileNameMaxLen);
    setIsCloseConn(true);
}

void HttpResponse::doResponse(HttpRequest &req, Buffer& inBuf, Buffer& outBuf) {
    version_ = req.getVersionString();
    string keep = req.getHeader("Connection");
    if (keep == "keep-alive" || keep == "Keep-Alive") {
        cout << "setIsCloseConn false" << endl;
        setIsCloseConn(false);
    } else {
        setIsCloseConn(true);
    }
    strcpy(realFile_, docRoot_);
    int len = strlen(docRoot_);
    string u = req.getUrl();
    cout << "url=" << u << endl;
    size_t found = u.rfind("/");
    string fileName = u.substr(found);

    if (fileName == "/2CGISQL.cgi" || fileName == "/3CGISQL.cgi") {  // 2是登录 3是注册
        // 取出buffer中的账号密码
        string content(inBuf.beginRead(), inBuf.beginRead()+inBuf.readableBytes());
        inBuf.retrieveAll();
        string name; 
        string passWd;
        for (int i = 0; i < content.length(); i++) {
            if (content[i] == '&') {
                name.assign(&content[5], &content[i]);
                passWd.assign(&content[i+10], &content[content.length()]);
                break;
            }
        }
        if (fileName == "/2CGISQL.cgi") {  // 登录则查看是否有对应的用户
            if (users.find(name) != users.end() && users[name] == passWd) {
                strcpy(url_, "/welcome.html");
            } else {
                strcpy(url_, "/logError.html");
            }
        } else {  // 注册则查看是否有重名的，没有重名则新增数据
            char sqlInsert[200];
            strcpy(sqlInsert, "INSERT INTO user(username, passwd) VALUES(");
            strcat(sqlInsert, "'");
            strcat(sqlInsert, name.c_str());
            strcat(sqlInsert, "', '");
            strcat(sqlInsert, passWd.c_str());
            strcat(sqlInsert, "')");

            if (users.find(name) == users.end()) {
                int res;
                {   
                    ConnectionPoolRAII connPoolRAII(&mysql_, connPool_);
                    MutexLockGuard guard(responseMtx);
                    res = mysql_query(mysql_, sqlInsert);
                    users[name] = passWd;
                }
                
                if (!res) {
                    strcpy(url_, "/log.html");
                } else {
                    strcpy(url_, "/registerError.html");
                }
            } else {  // 注册有同名
                strcpy(url_,  "/registerError.html");
            }

        }

    }

    if (fileName == "/0") {  // 注册页面
        strcpy(realFile_ + len, "/register.html");
    } else if (fileName == "/1") {  // 登录页面
        strcpy(realFile_ + len, "/log.html");
    } else if (fileName == "/5") {  // 请求图片
        strcpy(realFile_ + len, "/picture.html");
    } else if (fileName == "/6") {  // 请求视频
        strcpy(realFile_ + len, "/video.html");
    } else if (fileName == "/7") {  // 关注我
        strcpy(realFile_ + len, "/fans.html");
    } else if (fileName == "/") {
        strcpy(realFile_ + len, "/judge.html");
    } else if (fileName == "/favicon.ico") {
        strcpy(realFile_ + len, "/favicon.ico");
    } else {
        if (strlen(url_) == 0) {
            strcpy(url_, u.c_str());
        }
        strncpy(realFile_ + len, url_, strlen(url_));
    }
    
    cout << "realFile=" << realFile_ << endl;

    if (stat(realFile_, &fileStat_) < 0) {  // 没有该文件
        setHttpCode(HttpCode::k404NotFound);
    } else if (!(fileStat_.st_mode & S_IROTH)) {  // 其他人不可读
        setHttpCode(HttpCode::k403Forbidden);
    } else if (S_ISDIR(fileStat_.st_mode)) {  // 读取的是目录
        setHttpCode(HttpCode::k400BadRequest);
    } else {  // 已经过检查，可以读取文件
        setHttpCode(HttpCode::k200Ok);
    }
    addResponse(outBuf);
}

void HttpResponse::addResponse(Buffer& buf) {
    addResponseLine_(buf);
    addResponseHeader_(buf); 
    addBlankLine_(buf);
    addContent_(buf);
}

void HttpResponse::addResponseLine_(Buffer& buf) {
    // examples: HTTP/1.1 200 OK\r\n
    buf.append(version_.c_str(), version_.length());
    buf.append(" ", 1);
    buf.append(getHttpCodeString().c_str(), getHttpCodeString().length());
    buf.append("\r\n", 2);
}

void HttpResponse::addResponseHeader_(Buffer& buf) {

    // 1. 长连接
    if (isCloseConn_) {
        buf.append("Connection: close\r\n", 19);
    } else {
        cout << "buf.append keep-alive" << endl;
        buf.append("Connection: keep-alive\r\n", 24);
    }

    // 2. content-length
    string cl;
    if (httpCode_ == HttpCode::k200Ok) {
        cl = ("Content-Length: " + to_string(fileStat_.st_size) + "\r\n"); 
        buf.append(cl.c_str(), cl.length());
    } else if (httpCode_ == HttpCode::k400BadRequest){
        cl = ("Content-Length: " + to_string(strlen(kError400)) + "\r\n"); 
        buf.append(cl.c_str(), cl.length());
    } else if (httpCode_ == HttpCode::k403Forbidden) {
        cl = ("Content-Length: " + to_string(strlen(kError403)) + "\r\n"); 
        buf.append(cl.c_str(), cl.length());
    } else if (httpCode_ == HttpCode::k404NotFound) {
        cl = ("Content-Length: " + to_string(strlen(kError404)) + "\r\n"); 
        buf.append(cl.c_str(), cl.length());
    } else if (httpCode_ == HttpCode::k500InternalServerError) {
        cl = ("Content-Length: " + to_string(strlen(kError500)) + "\r\n"); 
        buf.append(cl.c_str(), cl.length());
    }
    
    // 3. content-type
    string ct = ("Content-Type: " + string("text/html") + "\r\n");
    buf.append(ct.c_str(), ct.length()); 

}


void HttpResponse::addContent_(Buffer& buf) {
    if (httpCode_ == HttpCode::k400BadRequest) {
        buf.append(kError400, strlen(kError400));
    } else if (httpCode_ == HttpCode::k403Forbidden) {
        buf.append(kError403, strlen(kError403));
    } else if (httpCode_ == HttpCode::k404NotFound) {
        buf.append(kError404, strlen(kError404));
    } else if (httpCode_ == HttpCode::k500InternalServerError) {
        buf.append(kError500, strlen(kError500));
    } else {
        return;
    }
}

void HttpResponse::addBlankLine_(Buffer& buf) {
    buf.append("\r\n", 2);
}

void HttpResponse::setHttpCode(HttpCode httpCode) {
    httpCode_ = httpCode;
}

string HttpResponse::getHttpCodeString() const {
    switch (httpCode_) {
    case HttpCode::k200Ok:
        return string("200 OK");
    case HttpCode::k300MovedPermanently:
        return string("300 Moved Permanently");
    case HttpCode::k400BadRequest:
        return string("400 Bad Request");
    case HttpCode::k403Forbidden:
        return string("403 Forbidden");
    case HttpCode::k404NotFound:
        return string("404 Not Found");
    case HttpCode::k500InternalServerError:
        return string("500 Internal Server Error");
    default:
        return string();
    }
}

HttpResponse::HttpCode HttpResponse::getHttpCode() const {
    return httpCode_;
}


const char* HttpResponse::getRealFile() const {
    return realFile_;
}

const struct stat& HttpResponse::getFileStat() const {
    return fileStat_;
}

bool HttpResponse::getIsCloseConn() const {
    return isCloseConn_;
}

void HttpResponse::setIsCloseConn(bool isClose) {
    isCloseConn_ = isClose;
}