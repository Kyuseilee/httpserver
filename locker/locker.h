/*
 * @Author: rosonlee 
 * @Date: 2021-03-23 18:46:21 
 * @Last Modified by: rosonlee
 * @Last Modified time: 2021-03-31 10:05:52
 */

#ifndef LOCKER_H
#define LOCKER_H

#include <exception>
#include <pthread.h>
#include <semaphore.h>

class sem{
public:
    sem(){
        if(sem_init( &m_sem_, 0, 0) != 0){
            throw std::exception();
        }
    }
    sem(int num){
        if (sem_init(&m_sem_, 0, num) != 0){
            throw std::exception();
        }
    }
    ~sem(){
        sem_destroy(&m_sem_);
    }
    bool Wait(){
        return sem_wait(&m_sem_) == 0;
    }
    bool Post(){
        return sem_post(&m_sem_) == 0;
    }
private:
    sem_t m_sem_;
};

class locker{
public:
    locker(){
        if(pthread_mutex_init( &m_mutex_, nullptr) != 0){
            throw std::exception();
        }
    }
    ~locker(){
        pthread_mutex_destroy( &m_mutex_);
    }
    bool Lock(){
        return pthread_mutex_lock(&m_mutex_) == 0;
    }
    bool Unlock(){
        return pthread_mutex_unlock(&m_mutex_) == 0;
    }
    pthread_mutex_t *get(){
        return &m_mutex_;
    }
private:
    pthread_mutex_t m_mutex_;
};

class cond{
public:
    cond(){
        if (pthread_cond_init(&m_cond_, nullptr) != 0)
            throw std::exception();
    }
    ~cond(){
        pthread_cond_destroy(&m_cond_);
    }
    bool Wait(pthread_mutex_t *m_mutex){
        int ret = 0;
        ret = pthread_cond_wait(&m_cond_, m_mutex);
        return ret == 0;
    }
    bool TimeWait(pthread_mutex_t *m_mutex, struct timespec t){
        int ret = 0;
        ret = pthread_cond_timedwait(&m_cond_, m_mutex, &t);
        return ret == 0;
    }
    bool Signal(){
        return pthread_cond_signal(&m_cond_) == 0;
    }
    bool Broadcast(){
        return pthread_cond_broadcast(&m_cond_) == 0;
    }
private:
    pthread_cond_t m_cond_;
};

#endif // LOCKER_H