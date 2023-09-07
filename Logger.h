#ifndef HTTPSERVER_LOGGER_H
#define HTTPSERVER_LOGGER_H

#include <stdio.h>
#include <string>

// 日志类
// 
// 实现和设计要点：
// (1) 日志类应该是一个单例（自然不可拷贝），全局使用一个。
// (2) 日志的输出
// 可以定制格式
// 且具有不同级别（如INFO、DEBUG、ERROR、FATAL等）
// 可以定制不同的输出位置（如文件、控制台等） 
class Logger {
public:
    enum class Level {
        INFO,
        DEBUG,
        ERROR,
        FATAL,
    };
 
    // 懒汉式单例模式
    static Logger& getInstance();

    // 设置日志级别
    void setLevel(Level level);

    // 主要方法进行log
    void log(const std::string& msg);

private:
    Logger() = default;
    ~Logger() = default;
    
    Level level_;
};


#define LOG_INFO(logMsgFormat, ...)                       \
    do {                                                  \
        Logger& logger = Logger::getInstance();           \
        logger.setLevel(Logger::Level::INFO);             \
        char buf[1024] = {0};                             \
        snprintf(buf, 1024, logMsgFormat, ##__VA_ARGS__); \
        logger.log(buf);                                  \
    } while (0)                                           \

#define LOG_ERROR(logMsgFormat, ...)                      \
     do {                                                 \
        Logger& logger = Logger::getInstance();           \
        logger.setLevel(Logger::Level::ERROR);            \
        char buf[1024] = {0};                             \
        snprintf(buf, 1024, logMsgFormat, ##__VA_ARGS__); \
        logger.log(buf);                                  \
    } while (0)                                           \

#define LOG_FATAL(logMsgFormat, ...)                      \
    do {                                                  \
        Logger& logger = Logger::getInstance();           \
        logger.setLevel(Logger::Level::FATAL);            \
        char buf[1024] = {0};                             \
        snprintf(buf, 1024, logMsgFormat, ##__VA_ARGS__); \
        logger.log(buf);                                  \
        exit(-1);                                         \
    } while (0)                                           \

#define LOG_DEBUG(logMsgFormat, ...)                      \
    do {                                                  \
        Logger& logger = Logger::getInstance();           \
        logger.setLevel(Logger::Level::DEBUG);            \
        char buf[1024] = {0};                             \
        snprintf(buf, 1024, logMsgFormat, ##__VA_ARGS__); \
        logger.log(buf);                                  \
    } while (0)                                           \


#endif  // HTTPSERVER_LOGGER_H