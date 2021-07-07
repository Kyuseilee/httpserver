/*
 * @Author: rosonlee 
 * @Date: 2021-07-05 19:26:12 
 * @Last Modified by: rosonlee
 * @Last Modified time: 2021-07-05 20:05:35
 */

/*
    ! 阻塞队列用queue实现，提供几个queue结构本身的函数封装接和几个操作接口
    ! 线程间通信使用锁和条件变量实现
    ! 附带了用互斥锁和条件变量实现的信号量
*/

#ifndef BLOCK_QUEUE_H
#define BLOCK_QUEUE_H

#include <condition_variable>
#include <mutex>
#include <sys/time.h>
#include <queue>

template<class T>
class BlockQueue{
public:
    explicit BlockQueue(size_t Maxcapacity = 1000);
    ~BlockQueue();

    void clear();

    bool empty();
    bool isfull();

    size_t size();

    void push(const T &item);
    bool pop(T &item);
    bool pop(T &item, int timeout);

    T front();
    T back();


    void Close();
    void flush();

private:
    queue<T>que_;
    size_t capacity_;
    std::mutex mtx_;
    bool isClose_;
    std::condition_variable condConsumer_;
    std::condition_variable condProducer_;
};

template<class T>
BlockQueue<T>::BlockQueue(size_t Maxcapacity) : capacity_(Maxcapacity){
    assert(Maxcapacity > 0);
    isClose_ = false;
}

template<class T>
BlockQueue<T>::~BlockQueue(){
    Close();
}

template<class T>
void BlockQueue<T>::Close(){
    {
        std::lock_guard<std::mutex>locker(mtx_);
        que_.clear();
        isClose_ = true;
    }
    condProducer_.notify_all();
    condConsumer_.notify_all();
}

template<class T>
bool BlockQueue<T>::empty(){
    std::lock_guard<std::mutex>locker(mtx_);
    return que_.empty();
}

template<class T>
bool BlockQueue<T>::isfull(){
    std::lock_guard<std::mutex>locker(mtx_);
    return que_.size() >= capacity_;
}

template<class T>
size_t size(){
    std::lock_guard<std::mutex>locker(mtx_);
    return que_.size();
}

template<class T>
void BlockQueue<T>::push(const T &item){
    {
        std::unique_lock<std::mutex>locker(mtx_);
        while (isfull()){
            condProducer_.wait(locker);
        }
        que_.emplace(item);
        condConsumer_.notify_one();
    }
}

template<class T>
bool BlockQueue<T>::pop(T &item){
    std::unique_lock<std::mutex>locker(mtx_);
    while (empty()){
        condConsumer_.wait(locker);
        if(isClose_)[
            isClose_ = false;
        ]
    }
    item = que_.front();
    que_.pop();
    condProducer_.notify_one();
    return true;
}

template<class T>
bool BlockQueue<T>::pop(T &item, int timeout){
    std::unique_lock<std::mutex>locker(mtx_);
    while(empty()){
        if (condConsumer_.wait_for(locker, std::chrono::seconds(timeout)) == std::cv_status::timeout){
            return false;
        }
        if (isClose_){
            isClose_ = false;
        }
    }
    item = que_.front();
    que_.pop();
    return true;
}

template<class T>
T BlockQueue<T>::front(){
    std::lock_guard<std::mutex>locker(mtx_);
    return que_.front();
}

template<class T>
T BlockQueue<T>::back(){
    std::lock_guard<std::mutex>locker(mtx_);
    return que_.back();
}

template<class T>
void BlockQueue<T>::flush(){
    std::lock_guard<std::mutex>locker(mtx_);
    condConsumer_.notify_one();
}

#endif // BLOCK_QUEUE_H