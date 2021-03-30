/*
 * @Author: rosonlee 
 * @Date: 2021-03-30 12:02:16 
 * @Last Modified by: rosonlee
 * @Last Modified time: 2021-03-30 19:34:10
 */
#ifndef BLOCK_QUEUE_H
#define BLOCK_QUEUE_H

#include <iostream>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>
#include "../locker/locker.h"
using namespace std;

template <class T>
class block_queue{
public:
    block_queue(int max_size = 1000){
        if (max_size <= 0){
            exit(-1);
        }

        m_max_size_ = max_size;
        m_array_ = new T[max_size];
        m_size_ = 0;
        m_front_ = -1;
        m_back_ = -1;
    }

    void Clear(){
        m_mutex_.Lock();
        m_size_ = 0;
        m_front_ = -1;
        m_back_ = -1;
        m_mutex_.Unlock();
    }

    ~block_queue(){
        m_mutex_.Lock();
        if (m_array_ != nullptr){
            delete [] m_array_;
        }
        m_mutex_.Unlock();
    }

    bool Full(){
        m_mutex_.Lock();
        if (m_size_ >= m_max_size_){
            m_mutex_.Unlock();
            return true;
        }

        m_mutex_.Unlock();
        return false;
    }

    bool Empty(){
        m_mutex_.Lock();
        if (m_size_ == 0){
            m_mutex_.Unlock();
            return true;
        }
        m_mutex_.Unlock();
        return false;
    }

    bool Front(T &value){
        m_mutex_.Lock();
        if (m_size_ == 0){
            m_mutex_.Unlock();
            return false;
        }

        value = m_array_[m_front_];
        m_mutex_.Unlock();
        return true;
    }
    bool Back(T &value){
        m_mutex_.Lock();
        if (m_size_ == 0){
            m_mutex_.Unlock();
            return false;
        }

        value = m_array_[m_back_];
        m_mutex_.Unlock();
        return true;
    }

    int Size(){
        int tmp = 0;

        m_mutex_.Lock();
        tmp = m_size_;
        m_mutex_.Unlock();

        return tmp;
    }

    int Maxsize(const T &item){
        int tmp = 0;

        m_mutex_.Lock();
        tmp = m_max_size_;
        m_mutex_.Unlock();

        return tmp;
    }

    bool Push(const T &item){
        m_mutex_.Lock();

        if (m_size_ >= m_max_size_){
            m_cond_.Broadcast();
            m_mutex_.Unlock();
            return false;
        }

        m_back_ = (m_back_ + 1) % m_max_size_;
        m_array_[m_back_] = item;

        m_size_++;

        m_cond_.Broadcast();
        m_mutex_.Unlock();

        return true;
    }

    bool Pop(T &item){
        m_mutex_.Lock();

        while (m_size_ <= 0){
            if (!m_cond_.Wait(m_mutex_.get())){
                m_mutex_.Unlock();
                return false;
            }
        }

        m_front_ = (m_front_ + 1) % m_max_size_;

        item = m_array_[m_front_];
        m_size_--;
        m_mutex_.Unlock();
        return true;
    }

    bool Pop(T &item, int ms_timeout){
        struct timespec t = {0, 0};
        struct timeval now = {0, 0};
        gettimeofday(&now, nullptr);
        m_mutex_.Lock();
        if (m_size_ <= 0){
            t.tv_sec = now.tv_sec + ms_timeout / 1000;
            t.tv_nsec = (ms_timeout % 1000) % 1000;
            if (!m_cond_.TimeWait(m_mutex_.get(), t)){
                m_mutex_.Unlock();
                return false;
            }
        }

        if (m_size_ <= 0){
            m_mutex_.Unlock();
            return false;
        }

        m_front_ = (m_front_ + 1) % m_max_size_;
        item = m_array_[m_front_];
        m_size_--;
        m_mutex_.Unlock();
        return true;
    }
private:
    locker m_mutex_;
    cond m_cond_;
    T *m_array_;
    int m_size_;
    int m_max_size_;
    int m_front_;
    int m_back_;
};

#endif // BLOCK_QUEUE_H
