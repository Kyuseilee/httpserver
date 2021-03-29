/*
 * @Author: rosonlee 
 * @Date: 2021-03-29 19:04:59 
 * @Last Modified by: rosonlee
 * @Last Modified time: 2021-03-29 19:50:14
 */

#ifndef TIMER_H
#define TIMER_H

#include<unistd.h>
#include<signal.h>
#include<sys/types.h>
#include<sys/epoll.h>
#include<fcntl.h>
#include<sys/uio.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<assert.h>
#include<sys/stat.h>
#include<string.h>
#include<pthread.h>
#include<stdio.h>
#include<stdlib.h>
#include<sys/mman.h>
#include<stdarg.h>
#include<errno.h>
#include<time.h>

class util_timer;

struct client_data{
    sockaddr_in address_;
    int sockfd_;
    util_timer *timer_;
};

class util_timer{
public:
    util_timer() : prev_(nullptr) , next_(nullptr) {}

public:
    time_t expire_;
    void (* CbFunc)(client_data *);
    client_data *user_data_;
    util_timer *prev_;
    util_timer *next_;
};

class sort_timer_lst{
public:
    sort_timer_lst();
    ~sort_timer_lst();

    void AddTimer(util_timer *timer);
    void AdjustTimer(util_timer *timer);
    void DelTimer(util_timer *timer);
    void Tick();
private:
    void __AddTimer(util_timer *timer, util_timer *lst_head);
    util_timer *head_;
    util_timer *tail_;
};

class Utils{
public:
    Utils(){};
    ~Utils(){};

    void Init(int timeslot);
    int SetNonBlocking(int fd);
    void AddFd(int epoll_fd, int fd, bool one_shot);

    static void SigHandler(int sig);
    void AddSig(int sig, void(handler)(int), bool restart = true);

    void TimerHandler();

    void ShowError(int conn_fd, const char* info);

public:
    static int *u_pipe_fd_;
    sort_timer_lst m_timer_lst_;
    static int u_epoll_fd_;
    int m_TIMESLOT_;
};

void CbFunc(client_data *user_data);

#endif // TIMER_H