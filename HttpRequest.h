#ifndef HTTPSERVER_HTTPREQUEST_H
#define HTTPSERVER_HTTPREQUEST_H

#include "Buffer.h"
#include <map>
#include <string>


class HttpRequest {
public:
    enum class CheckState {
        kCheckRequestLine = 0,
        kCheckRequestHead,
        kCheckRequestBody,
        kFinished
    };

    enum class Method {
        kGet = 0,
        kPost
    };

    enum class Version {
        kHttp10 = 0,
        kHttp11
    };
    
    enum class HttpCode {
        kNoRequest = 0,
        kBadRequest,
        kInternalError
    };

    HttpRequest();
    void reset(HttpRequest& other);
    bool parse(Buffer& buf);
    bool gotAll() const;

    void setMethod(const char* begin, const char* end);
    
    void setVersion(Version v);
    Version getVersion() const;
    std::string getVersionString() const;
    void setUrl(const char* begin, const char* end);
    std::string getUrl() const;

    void addHeader(const char* begin, const char* colon, const char* end);
    std::string getHeader(const std::string& field) const;

    void printHeaders() const; // for debug

private:
    bool parseRequestLine_(const char* begin, const char* end);

    CheckState checkState_;
    Method method_;
    Version version_;
    std::string url_;

    std::map<std::string, std::string> headers_;



};

#endif  // 