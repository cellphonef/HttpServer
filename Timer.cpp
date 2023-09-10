#include "Timer.h"
#include <sys/time.h>
#include <iostream>


using namespace std;


TimerHeap::~TimerHeap() {
    for(int i = 0; i < timers_.size(); i++) {
        delete timers_[i];
    }
}

void TimerHeap::addTimer(int connFd, int64_t expired, TimerCallback callback) {
    TimerNode* timer = new TimerNode;
    timer->fd = connFd;
    timer->callback = callback;
    struct timeval tv;
    gettimeofday(&tv, 0);
    int64_t now = tv.tv_sec * 1000 + tv.tv_usec / 1000;  // 当前毫秒数
    timer->expired = now + expired;  // 过期时间，绝对时间
    timers_.push_back(timer);
    int len = timers_.size();
    int index = siftUp_(len-1);
    fd2Idx_[connFd] = index;
}

void TimerHeap::adjustTimer(int fd) {
    int index = fd2Idx_[fd];
    struct timeval tv;
    gettimeofday(&tv, 0);
    int64_t now = tv.tv_sec * 1000 + tv.tv_usec / 1000;
    timers_[index]->expired = now + kTimerSlot;
    siftDown_(index, timers_.size());
}



void TimerHeap::delTimer(int fd) {
    int index = fd2Idx_[fd];
    timers_[index]->callback();
    del_(index);
    fd2Idx_.erase(fd);
}

void TimerHeap::del_(int index) {
    int len = timers_.size();
    swapNode_(index, len-1);  // 先与最后一个节点交换
    if (index == len-1) {
        timers_.pop_back();
        return;
    }
    int i = index;
    int j = (index - 1) / 2;
    if (timers_[i] >= timers_[j]) {
        siftDown_(i, len-1);
    } else {
        siftUp_(i);
    }
    timers_.pop_back();

}


void TimerHeap::tick() {
    if (timers_.empty()) {
        return;
    }
    while (!timers_.empty()) {
        TimerNode* timer = timers_.front();
        struct timeval tv;
        gettimeofday(&tv, 0);
        int64_t now = tv.tv_sec * 1000 + tv.tv_usec / 1000;
        if (timer->expired > now) {
            break;
        }
        timer->callback();
        del_(0);

    }


}

int64_t TimerHeap::getNextTick() {
    tick();
    int64_t res = -1;
    if (timers_.empty()) {
        return res;
    }
    while (res <= 0 && !timers_.empty()) {
        struct timeval tv;
        gettimeofday(&tv, 0);
        int64_t now = tv.tv_sec * 1000 + tv.tv_usec / 1000;
        res = timers_.front()->expired - now;  
        if (res <= 0) {
            tick();
        }
    }
    if (res <= 0) {
        return -1;
    }  
    return res;
    
}

int TimerHeap::siftUp_(int index) {
    int i = index;
    int j = (index - 1) / 2;
    while (i > 0) {
        if (timers_[i]->expired >= timers_[j]->expired) {
            break;
        }
        swapNode_(i, j);
        i = j;
        j = (i - 1) / 2;
    }
    return i;
}


int TimerHeap::siftDown_(int index, int len) {
    int i = index;
    int left = 2 * i + 1;
    int right = 2 * i + 2;
    while (left < len || right < len) {
        int minIndex;
        if (right < len) {
            minIndex = (timers_[left]->expired < timers_[right]->expired ? left : right);
        } else {
            minIndex = left;
        }
        if (timers_[i] <= timers_[minIndex]) {
            break;
        }

        swapNode_(i, minIndex);
        i = minIndex;
        left = 2 * i + 1;
        right = 2 * i + 2;
    }
    return i;

}

void TimerHeap::swapNode_(int lhs, int rhs) {
    if (lhs == rhs) {
        return;
    }

    TimerNode* tmp = timers_[lhs];
    timers_[lhs] = timers_[rhs];
    timers_[rhs] = tmp;

    // note  还要改变index
    fd2Idx_[timers_[lhs]->fd] = lhs;
    fd2Idx_[timers_[rhs]->fd] = rhs;

}
