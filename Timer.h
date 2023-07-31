#ifndef HTTPSERVER_TIMER_H
#define HTTPSERVER_TIMER_H

#include <stdint.h>
#include <vector>
#include <map>
#include <functional>
#include <memory>

const int kTimerSlot = 60 * 1000;

typedef std::function<void()> TimerCallback;

struct TimerNode {
    int fd;
    int64_t expired;
    TimerCallback callback;
};


class TimerHeap {
public:
    typedef std::shared_ptr<TimerHeap> ptr;
    
    TimerHeap()=default;
    ~TimerHeap();

    void addTimer(int connFd, int64_t interval, TimerCallback callback);
    void adjustTimer(int fd);
    void delTimer(int fd);
    

    void tick();

    int64_t getNextTick();

private:
    void del_(int index);

    int siftUp_(int index);  // 上浮
    int siftDown_(int index, int len);  // 下沉
    void swapNode_(int lhs, int rhs);  // 交换节点


    std::map<int, int> fd2Idx_;
    std::vector<TimerNode*> timers_;
};





#endif // HTTPSERVER_TIMEQUEUE_H