#include "Logger.h"
#include "Thread.h"

#include <iostream>

Logger& Logger::getInstance() {
    static Logger logger;
    return logger;
}

void Logger::setLevel(Level level) {
    level_ = level;
}

// [级别信息] 时间戳 : msg 
void Logger::log(const std::string& msg) {
    std::string pre = "";
    switch(level_) {
        case Level::INFO:
            pre = "[INFO] ";
            break;
        case Level::DEBUG:
            pre = "[DEBUG] ";
            break;
        case Level::ERROR:
            pre = "[ERROR] ";
            break;
        case Level::FATAL:
            pre = "[FATAL] ";
            break;
        default:
            break;
    }
    time_t now; 
    time(&now);
    std::string timeString(ctime(&now));
    timeString = timeString.substr(0, timeString.size() - 1);


    std::cout << pre + timeString << " " << gettid() << " : " << msg << std::endl;
    
}