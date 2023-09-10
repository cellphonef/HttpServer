#ifndef HTTPSERVER_THREADPOOL_H
#define HTTPSERVER_THREADPOOL_H


#include "Mutex.h"
#include "MutexLockGuard.h"
#include "ConditionVar.h"
#include "Thread.h"
#include "Logger.h"
#include <queue>

#include <iostream>

using namespace std;


template <class T>
class ThreadPool {
public:
    ThreadPool(int threadNum);
    ~ThreadPool();

    static void* threadFunc(void* arg);
    void run();
    void start();
    void submit(T* task);

    void quit();


private:
    // 是否停止线程池
    bool stop_;

    // 用于保护任务队列
    Mutex mtx_;
    ConditionVar cond_;
    
    // 任务队列
    std::queue<T*> taskQueue_;

    // 工作线程
    int threadNum_;
    Thread* workers_;


};

template <class T>
ThreadPool<T>::ThreadPool(int threadNum) : threadNum_(threadNum), stop_(true) {}

template <class T>
ThreadPool<T>::~ThreadPool() {
    delete []workers_;
}

template <class T>
void* ThreadPool<T>::threadFunc(void* arg) {
    ThreadPool* mine = (ThreadPool*)arg;
    mine->run();
}

template <class T>
void ThreadPool<T>::start() {
    stop_ = false;
    workers_ = new Thread[threadNum_];
    for (int i = 0; i < threadNum_; i++) {
        workers_[i].setFunc(threadFunc, this);
        workers_[i].start();
        LOG_INFO("The %d thread start!", i);
    }
}

template <class T> 
void ThreadPool<T>::run() {
    LOG_INFO("thread start running!");
    while (!stop_) {
        T* item;
        {
            MutexLockGuard guard(mtx_);
            while (taskQueue_.empty()) {
                cond_.wait(mtx_);
            }
            item = taskQueue_.front();
            taskQueue_.pop();
        }
        item->process();
    }  
}
    


template <class T>
void ThreadPool<T>::submit(T* item) {
    MutexLockGuard guard(mtx_);
    taskQueue_.push(item);
    cond_.signal();
}

template <class T>
void ThreadPool<T>::quit() {
    MutexLockGuard guard(mtx_);
    stop_ = true;
    cond_.signalAll();
}


#endif