#include "Mutex.h"
#include <iostream>
using namespace std;

Mutex::Mutex() {
    pthread_mutex_init(&mtx_, nullptr);
}

Mutex::~Mutex() {
    pthread_mutex_destroy(&mtx_);
}

void Mutex::lock() {
    pthread_mutex_lock(&mtx_);
}

void Mutex::unlock() {
    pthread_mutex_unlock(&mtx_);

}

pthread_mutex_t& Mutex::get(){
    return mtx_;
}