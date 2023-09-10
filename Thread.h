#ifndef HTTPSERVER_THREAD_H
#define HTTPSERVER_THREAD_H

#include <pthread.h>
#include <sys/syscall.h>
#include <unistd.h>


pid_t gettid();


class Thread {  //FIXME: C++11
public:
    typedef void*(*func_ptr)(void*);
    Thread() = default;
    Thread(func_ptr start_rountie, void* args);
    ~Thread();
    void start();
    void setFunc(func_ptr func, void* args);

    pthread_t gettid() const;



private:
    func_ptr start_rountie_ = nullptr;
    void* args_ = nullptr;
    pthread_t tid_;

};

#endif