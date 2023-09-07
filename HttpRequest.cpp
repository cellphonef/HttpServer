#include "HttpRequest.h"
#include <algorithm>
#include <iostream>

using namespace std;

HttpRequest::HttpRequest() : checkState_(CheckState::kCheckRequestLine) {}

void HttpRequest::reset(HttpRequest& other) {
    std::swap(checkState_, other.checkState_);
    std::swap(method_, other.method_);
    std::swap(version_, other.version_);
    url_.swap(other.url_);
    headers_.swap(other.headers_);
}


bool HttpRequest::parse(Buffer& buf) {
    bool ok = true;
    bool hasMore = true;
    while (hasMore) {  
        if (checkState_ == CheckState::kCheckRequestLine) {
            const char* crlf = buf.findCRLF();
            if (crlf) {
                ok = parseRequestLine_(buf.peek(), crlf);
                if (ok) {
                    buf.retrieveUtil(crlf + 2);
                    checkState_ = CheckState::kCheckRequestHead;
                } else {
                    hasMore = false;
                }
            } else {
                hasMore = false;
            }
        }
        else if (checkState_ == CheckState::kCheckRequestHead) {
            const char* crlf = buf.findCRLF();
            if (crlf) {
                const char* colon = std::find(buf.peek(), crlf, ':');
                if (colon != crlf) {
                    addHeader(buf.peek(), colon, crlf);
                } else {
                    if (headers_.find("Content-Length") != headers_.end()) {
                        int len = stoi(headers_["Content-Length"]);
                        if (len > 0) {
                            checkState_ = CheckState::kCheckRequestBody;
                        } else {
                            checkState_ = CheckState::kFinished;
                            hasMore = false;
                        }
                    } else {
                        checkState_ = CheckState::kFinished;
                        hasMore = false;
                    }
                }
                buf.retrieveUtil(crlf + 2);
            } else {
                hasMore = false;
            }
        }
        else if (checkState_ == CheckState::kCheckRequestBody) {
            int len = stoi(headers_["Content-Length"]);

            if (len > buf.readableBytes()) {
                hasMore = false;
            } else {
                checkState_ = CheckState::kFinished;
                hasMore = false;
            }
        }
    }
    return ok;
    
}

bool HttpRequest::parseRequestLine_(const char* begin, const char* end) {
    bool succeed = false;
    const char* start = begin;
    const char* space = find(start, end, ' ');

    if (space != end) {
        setMethod(start, space);
        start = space + 1;
        space = find(start, end, ' ');

        if (space != end) {
            const char* question = find(start, space, '?');
            if (question != space) {

            } else {
                setUrl(start, space);
            }
        }
        start = space + 1;
        succeed = ((end-start == 8) && equal(start, end-1, "HTTP/1."));
        if (succeed) {
            if (*(end-1) == '1') {
                setVersion(Version::kHttp11);
            } else if (*(end-1) == '0') {
                setVersion(Version::kHttp10);
            } else {
                succeed = false;
            }
        }
    }
    return succeed;
}


void HttpRequest::addHeader(const char* begin, const char* colon, const char* end) {
    // examples: 
    // host: www.xxx.com
    string field(begin, colon);
    colon++;
    while (colon < end && isspace(*colon)) {  // 去掉头部空白
        colon++;
    }
    string value(colon, end);
    while (!value.empty() && isspace(value[value.size()-1])) {  // 去掉尾部空白
        value.resize(value.size()-1);
    }
    headers_[field] = value;
}

string HttpRequest::getHeader(const string& field) const {
    string res;
    auto it = headers_.find(field);
    if (it != headers_.end()) {
        res = it->second;
    }
    return res;
}

bool HttpRequest::gotAll() const {
    return checkState_ == CheckState::kFinished;
}


void HttpRequest::setMethod(const char* begin, const char* end) {
    string m(begin, end);
    if (m == "GET") {
        method_ = Method::kGet;
    } else if (m == "POST") {
        method_ = Method::kPost;
    } else {

    }

}
    
void HttpRequest::setVersion(Version v) {
    version_ = v;
}

HttpRequest::Version HttpRequest::getVersion() const {
    return version_;
}

string HttpRequest::getVersionString() const {
    if (version_ == Version::kHttp10) {
        return string("HTTP/1.0");
    } else if (version_ == Version::kHttp11) {
        return string("HTTP/1.1");
    }
}

void HttpRequest::setUrl(const char* begin, const char* end) {
    url_.assign(begin, end);
}

string HttpRequest::getUrl() const {
    return url_;
}


void HttpRequest::printHeaders() const {
    for (const auto& header : headers_) {
        printf("%s=%s\n", header.first.c_str(), header.second.c_str());
    }
}