#ifndef HTTPSERVER_MUTEXLOCKGUARD_H
#define HTTPSERVER_MUTEXLOCKGUARD_H

#include "Mutex.h"
#include <iostream>

class MutexLockGuard {
public:
    MutexLockGuard(Mutex& mtx) : mtx_(mtx) {
        mtx_.lock();
    }
    ~MutexLockGuard() {
        mtx_.unlock();
    }

private:
    Mutex& mtx_;
};

#endif