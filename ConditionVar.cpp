#include "ConditionVar.h"


ConditionVar::ConditionVar() {
    pthread_cond_init(&cond, nullptr);
}


ConditionVar::~ConditionVar() {
    pthread_cond_destroy(&cond);
}

void ConditionVar::wait(Mutex& mtx) {
    pthread_cond_wait(&cond, &mtx.get());
}



void ConditionVar::signal() {
    pthread_cond_signal(&cond);
}

void ConditionVar::signalAll() {
    pthread_cond_broadcast(&cond);
}