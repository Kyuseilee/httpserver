/*
 * @Author: rosonlee 
 * @Date: 2021-03-23 18:44:00 
 * @Last Modified by: rosonlee
 * @Last Modified time: 2021-03-23 20:39:24
 */

#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <list>
#include <cstdio>
#include <exception>
#include <pthread.h>
#include "locker.h"

template <typename T>
class threadpool{
public:
    threadpool(int thread_number = 8, int max_requests = 10000);
    ~threadpool();
    bool Append(T* request);

private:
    static void* __Worker(void* arg);
    void __Run();
private:
    int m_thread_number_; 
    int m_max_requests_;
    pthread_t* m_threads_;
    std::list<T* > m_workqueue_;
    locker m_queuelocker_;
    sem m_queuestat_;
    bool m_stop_
};

template<typename T>
threadpool<T>::threadpool(int thread_number, int max_requests) : m_thread_number_(thread_number), m_max_requests_(max_requests), m_stop_(false), m_threads_(nullptr){
    if ((thread_number <= 0) || (max_requests <= 0)){
        throw std::exception();
    }
    m_threads_ = new pthread_t[m_thread_number_];
    if (!m_threads_){
        throw std::exception();
    }
    for (int i = 0; i < thread_number; i++){
        printf("create the %dth thread\n", i);
        if(pthread_create(m_threads_ + i, nullptr, __worker, this) != 0){
            delete[] m_threads_;
            throw std::exception();
        }
        if (pthread_detach(m_threads_[i])){
            delete[] m_threads_;
            throw std::exception();
        }
    }
}

template <typename T>
threadpool< T >::~threadpool(){
    delete[] m_threads_;
    m_stop_ = true;
}

template <typename T>
bool threadpool< T >::Append(T* request){
    m_queuelocker_.Lock();
    if(m_workqueue_.size() > m_max_requests_){
        m_max_requests_.Unlock();
        return false;
    }
    m_workqueue_.push_back(request);
    m_queuelocker_.Unlock();
    m_queuestat_.post();
    return true;
}

template<typename T>
void* threadpool< T >::__Worker(void* arg){
    thread_pool* pool = (threadpool*) arg;
    pool->__Run();
    return pool;
}

template<typename T>
void threadpool< T >::__Run(){
    while (!m_stop_){
        m_queuestat_.wait();
        m_queuelocker_.Lock();
        if (m_workqueue_.empty()){
            m_queuelocker_.Unlock();
            continue;
        }
        T* request = m_workqueue_.front();
        m_workqueue_.pop_front();
        m_queuelocker_.Unlock();
        if(!request){
            continue;
        }
        request->process();
    }
}
#endif // THREADPOOL_H