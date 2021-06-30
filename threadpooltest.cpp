/*
 * @Author: rosonlee 
 * @Date: 2021-06-29 20:56:22 
 * @Last Modified by: rosonlee
 * @Last Modified time: 2021-06-30 20:18:26
 */

#include "threadpool.h"
#include <iostream>
#include <string.h>
#include <unistd.h>

class Test{
public:
    Test(){};
    ~Test(){};
};
class Client{
public:
    Client(){};
private:
    int cnt = 0;
};

class work{
public:
    work():threadpool_(new ThreadPool(5)){
        num = 0;
    }
    ~work();
    void func(Client* client);
    void test();
    void start(){
        while (1){
            func(&c);
        }
    }

private:
    std::unique_ptr<ThreadPool> threadpool_;
    int num;
    Client c;
};

void work::func(Client* client){
    threadpool_->AddTask(std::bind(&work::test, this, client));
}

void work::test(){
    std::cout << "?????" << std::endl;
}

int main(){
    work w;
    w.start();
    return 0;
}
