#include "ConnectionPool.h"
#include "MutexLockGuard.h"

#include <iostream>
using namespace std;

ConnectionPool::~ConnectionPool() {
    destoryPool();
}


void ConnectionPool::init(string url, string userName, string passWd,
                          string databaseName, int port, int maxConnNum) {
    url_ = url;
    userName_ = userName;
    passWd_ = passWd;
    databaseName_ = databaseName;
    port_ = port;
    maxConnNum_ = maxConnNum;

    for (int i = 0; i < maxConnNum; i++) {
        MYSQL* con = NULL;
        con = mysql_init(con);
        if (con == NULL) {
            // FIXME: LOG
            exit(1);
        }
        con = mysql_real_connect(con, url_.c_str(), userName_.c_str(), passWd_.c_str(), 
                                 databaseName_.c_str(), port_, NULL, 0);
        if (con == NULL) {
            // FIXME: LOG
            exit(1);
        }
        conns_.push_back(con);   
    }

}


ConnectionPool* ConnectionPool::getInstance() {
    static ConnectionPool connPool;
    return &connPool;

}

MYSQL* ConnectionPool::getConn() {
    // 可能被多个线程访问，需要保证线程安全
    MutexLockGuard guard(mtx_);
    while (conns_.empty()) {
        cond_.wait(mtx_);
    }
    MYSQL* con;
    con = conns_[conns_.size()-1];
    conns_.pop_back();
    return con;
}

void ConnectionPool::releaseConn(MYSQL* conn) {
    MutexLockGuard guard(mtx_);
    conns_.push_back(conn);
    cond_.signal();
}


void ConnectionPool::destoryPool() {
    for (int i = 0; i < conns_.size(); i++) {
        mysql_close(conns_[i]);
    }
}



ConnectionPoolRAII::ConnectionPoolRAII(MYSQL** mysql, ConnectionPool* connPool) {
    *mysql = connPool->getConn();
    mysql_ = *mysql;
    connPool_ = connPool;
}

ConnectionPoolRAII::~ConnectionPoolRAII() {
    connPool_->releaseConn(mysql_);
}