/*
 * @Author: rosonlee 
 * @Date: 2021-06-17 21:47:39 
 * @Last Modified by: rosonlee
 * @Last Modified time: 2021-06-29 22:29:03
 */

/*
    ! 池子的设计思想:
        * 池子拥有多个线程，这些线程共享任务队列，私有抢占的函数
        * 对于线程池，其本身应该有线程间通信的三种方式：互斥锁、条件变量、信号量，同时还应该有一个运行标志（bool变量），最重要的，就是任务队列
        * 对于threadpool中的所有私有变量，可以使用一个结构体来集成, 然后开放一个指向该结构体的智能指针，对数据进行修改
    ! thread库的方法
        * 使用匿名函数直接在thread创建过程中绑定，原因是将work写为private变量然后在创建里绑定的时候会提示一个non static错误，搜到的解决方法要么把功能声明成static，要么使用匿名函数 
        * https://blog.csdn.net/weixin_30827565/article/details/96195835
    ! 构造函数
        * 使用explicit显式声明指定数字的含义，同时将空参的构造使用default指向编译器自行的constructor（如果写了含参而不写无参的constructor编译器会报错）
    ! std::forward
        *右值引用类型是独立于值的，一个右值引用参数作为函数的形参，在函数内部再转发该参数的时候它已经变成一个左值，并不是他原来的类型。需要一种方法能够按照参数原来的类型转发到另一个函数，这种转发类型称为完美转发。
*/

#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <mutex>
#include <condition_variable>
#include <queue>
#include <thread>
#include <functional>
#include <cassert>
class ThreadPool {
public:
    explicit ThreadPool(size_t threadCount = 8): pool_(std::make_shared<Pool>()) {
            assert(threadCount > 0);
            for(size_t i = 0; i < threadCount; i++) {
                std::thread([pool = pool_] {
                    std::unique_lock<std::mutex> locker(pool->mtx);
                    while(true) {
                        if(!pool->tasks.empty()) {
                            auto task = std::move(pool->tasks.front());
                            pool->tasks.pop();
                            locker.unlock();
                            task();
                            locker.lock();
                        } 
                        else if(pool->isClosed) break;
                        else pool->cond.wait(locker);
                    }
                }).detach();
            }
    }

    ThreadPool() = default;

    ThreadPool(ThreadPool&&) = default;
    
    ~ThreadPool() {
        if(static_cast<bool>(pool_)) {
            {
                std::lock_guard<std::mutex> locker(pool_->mtx);
                pool_->isClosed = true;
            }
            pool_->cond.notify_all();
        }
    }

    template<class F>
    void AddTask(F&& task) {
        {
            std::lock_guard<std::mutex> locker(pool_->mtx);
            pool_->tasks.emplace(std::forward<F>(task));
        }
        pool_->cond.notify_one();
    }

private:
    struct Pool {
        std::mutex mtx;
        std::condition_variable cond;
        bool isClosed;
        std::queue<std::function<void()>> tasks;
    };
    std::shared_ptr<Pool> pool_;
};


#endif //THREADPOOL_H