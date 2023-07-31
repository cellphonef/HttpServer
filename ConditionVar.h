#ifndef HTTPSERVER_BASE_CONDITIONVAR_H
#define HTTPSERVER_BASE_CONDITIONVAR_H

#include "Mutex.h"
#include <pthread.h>

class ConditionVar {
public:
    ConditionVar();
    ConditionVar(const ConditionVar& other) = delete;
    ~ConditionVar();
    ConditionVar& operator=(const ConditionVar& other) = delete;

    void wait(Mutex& mtx);
    void signal();
    void signalAll();

private:
    pthread_cond_t cond;

};

#endif