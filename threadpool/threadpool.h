/*
 * @Author: rosonlee 
 * @Date: 2021-03-23 18:44:00 
 * @Last Modified by: rosonlee
 * @Last Modified time: 2021-03-30 19:35:17
 */

#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <list>
#include <cstdio>
#include <exception>
#include <pthread.h>
#include "../locker/locker.h"
#include "../Sql/sql_connection_pool.h"

template <typename T>
class thread_pool{
public:
    thread_pool(connection_pool *connPool, int thread_number = 8, int max_requests = 10000);
    ~thread_pool();
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
    bool m_stop_;
    connection_pool *m_connPool;
};

template<typename T>
thread_pool<T>::thread_pool(connection_pool *connPool, int thread_number, int max_requests) :m_thread_number_(thread_number), m_max_requests_(max_requests), m_stop_(false), m_threads_(nullptr), m_connPool(connPool){
    if ((thread_number <= 0) || (max_requests <= 0)){
        throw std::exception();
    }
    m_threads_ = new pthread_t[m_thread_number_];
    if (!m_threads_){
        throw std::exception();
    }
    for (int i = 0; i < thread_number; i++){
        printf("create the %dth thread\n", i);
        if(pthread_create(m_threads_ + i, NULL, __Worker, this) != 0){
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
thread_pool< T >::~thread_pool(){
    delete[] m_threads_;
    m_stop_ = true;
}

template <typename T>
bool thread_pool< T >::Append(T* request){
    m_queuelocker_.Lock();
    if(m_workqueue_.size() >= m_max_requests_){
        m_queuelocker_.Unlock();
        return false;
    }
    m_workqueue_.push_back(request);
    m_queuelocker_.Unlock();
    m_queuestat_.Post();
    return true;
}

template<typename T>
void* thread_pool< T >::__Worker(void* arg){
    thread_pool* pool = (thread_pool*) arg;
    pool->__Run();
    return pool;
}

template<typename T>
void thread_pool< T >::__Run(){
    while (1){
        m_queuestat_.Wait();
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
        request->Process();
    }
}
#endif // THREADPOOL_H