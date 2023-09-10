#include "Thread.h"

__thread pid_t tid = 0;

pid_t gettid() {
    if (tid == 0)
        tid = syscall(SYS_gettid);
    return tid;
}

Thread::Thread(func_ptr start_rountie,void* args) : start_rountie_(start_rountie), args_(args){}

void Thread::start() {
    pthread_create(&tid_, nullptr, start_rountie_, args_);
    pthread_detach(tid_);
}


Thread::~Thread(){
}


pthread_t Thread::gettid() const {
    return tid_;
}


void Thread::setFunc(func_ptr func, void* args) {
    start_rountie_ = func;
    args_ = args;
}

