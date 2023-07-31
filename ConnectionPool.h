#ifndef HTTPSERVER_CONNECTIONPOOL_H
#define HTTPSERVER_CONNECTIONPOOL_H


#include "Mutex.h"
#include "ConditionVar.h"
#include <mysql/mysql.h>


#include <string>
#include <vector>
  
/**
 * 单例模式连接池   
 * 
*/
class ConnectionPool {
public:
    ConnectionPool() = default;
    ~ConnectionPool();
    void init(std::string url, std::string userName, std::string passWd,
              std::string databaseName, int port, int maxConnNum);

    static ConnectionPool* getInstance();

    MYSQL* getConn();
    void releaseConn(MYSQL* conn);
    void destoryPool();



private:
    std::string url_;
    std::string userName_;
    std::string passWd_;
    std::string databaseName_;
    int port_;
    int maxConnNum_;  // 最大连接数量
    std::vector<MYSQL*> conns_;

    Mutex mtx_;
    ConditionVar cond_;

};


class ConnectionPoolRAII {
public:
    ConnectionPoolRAII(MYSQL** mysql, ConnectionPool* connPool);
    ~ConnectionPoolRAII();

private:
    MYSQL* mysql_;
    ConnectionPool* connPool_;
};

#endif  // HTTPSERVER_CONNECTIONPOOL_H