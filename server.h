/*
 * @Author: rosonlee 
 * @Date: 2021-03-22 19:51:41 
 * @Last Modified by: rosonlee
 * @Last Modified time: 2021-03-29 20:59:25
 */

#ifndef SERVER_H
#define SERVER_H

#include <iostream>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <sys/socket.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <cassert>
#include <sys/epoll.h>
#include <sys/sendfile.h>
#include<unistd.h>
#include<exception>
#include<cstdio>
#include "http/http_conn.h"
#include "./locker/locker.h"
#include "./threadpool/threadpool.h"
#include "./timer/timer.h"

const int MAX_FD = 65536;
const int  MAX_EVENT_NUMBER = 10000;
const int TIMESLOT = 5;

using namespace std;

class Server{
public:
    Server();
    ~Server();
    void Init();
    void InitThreadPool();
    void Listen();
    void Loop();

    void Timer(int conn_fd, struct sockaddr_in client_address);
    void AdjustTimer(util_timer *timer);

    void HandleTimer(util_timer *timer, int fd);
    bool HandleConnect();
    void HandleWrite(int fd);
    void HandleRead(int fd);
    bool HandleSignal(bool &timeout, bool &stop_server);


public:
    Utils utils;
    int pipe_fd_[2];
    client_data *users_timer;
    thread_pool<http_conn>*m_pool_;
    http_conn* users;
    int user_count = 0;
    int listen_fd_, epoll_fd_;
    int port_;
    epoll_event events_[MAX_EVENT_NUMBER];


};

#endif // SERVER_H