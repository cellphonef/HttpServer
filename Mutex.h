#ifndef HTTPSERVER_MUTEX_H
#define HTTPSERVER_MUTEX_H

#include <pthread.h>

class Mutex {
public:
    Mutex();
    Mutex(const Mutex& other) = delete;
    ~Mutex();

    void lock();
    void unlock();
    pthread_mutex_t& get();

    Mutex& operator=(const Mutex& other) = delete;


private:
    pthread_mutex_t mtx_;
};

#endif