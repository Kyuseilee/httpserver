/*
 * @Author: rosonlee 
 * @Date: 2021-06-30 19:07:37 
 * @Last Modified by: rosonlee
 * @Last Modified time: 2021-06-30 19:45:35
 */

/*
    ! 封装了Epoll类的常用功能
        ^ Epoll的主要函数：（epoll_create:(在构造函数中创建)，epoll_ctl(在三个功能函数中实现), epoll_wait（在Wait中实现）
    ! 主要参数：一个epoll数组的fd标识符，一个epoll数组
    ! 函数：
        & 构造与析构函数
        & 功能封装函数:往数组中增加Fd(Addfd)、从数组中移除fd(delfd)和修改对应fd的功能（Modfd)、阻塞等待函数（Wait)
        & 成员访问函数:Getfd、 GetEvents
    ! 函数参数：fd:整形， events：Epoll的事件，属于宏定义，unit32_t型
    数
        
*/

#ifndef EPOLLER_H
#define EPOLLER_H

#include <sys/epoll.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>
#include <vector>
#include <errno.h>

class Epoller
{
private:
    int epoll_fd_;
    std::vector<struct epoll_event>events_;
    
public:
    explicit Epoller(int maxEvent = 1024);
    ~Epoller();
    
    bool AddFd(int fd, uint32_t events);

    bool ModFd(int fd, uint32_t events);

    bool DelFd(int fd);

    int Wait(int timeoutMs);

public:
    int GetEventFd(size_t i) const ; 

    uint32_t GetEvents(size_t i) const;
    
};

#endif // EPOLLER_H