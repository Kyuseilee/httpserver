/*
 * @Author: rosonlee 
 * @Date: 2021-03-23 18:46:21 
 * @Last Modified by: rosonlee
 * @Last Modified time: 2021-03-23 20:39:54
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
private:
    pthread_mutex_t m_mutex_;
};

class cond{
public:
    cond(){
        if (pthread_mutex_init(&m_mutex_, nullptr) != 0)
            throw std::exception();
        if (pthread_cond_init(&m_cond_, nullptr) != 0){
            pthread_mutex_destroy( &m_mutex_);
            throw std::exception();
        }
    }
    ~cond(){
        pthread_mutex_destroy(&m_mutex_);
        pthread_cond_destroy(&m_cond_);
    }
    bool Wait(){
        int ret = 0;
        pthread_mutex_lock(&m_mutex_);
        ret = pthread_cond_wait(&m_cond_, &m_mutex_);
        pthread_mutex_unlock(&m_mutex_);
        return ret == 0;
    }
    bool Signal(){
        return pthread_cond_signal(&m_cond_) == 0;
    }
private:
    pthread_mutex_t m_mutex_;
    pthread_cond_t m_cond_;
};

#endif // LOCKER_H