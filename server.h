/*
 * @Author: rosonlee 
 * @Date: 2021-03-22 19:51:41 
 * @Last Modified by: rosonlee
 * @Last Modified time: 2021-03-30 20:27:01
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
    void Init(int port , string user, string passWord, string databaseName,int threadnum, int log_write ,int sql_num, int close_log);
    void InitThreadPool();
    void SqlPool();
    void LogWrite();


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
    int port_;
    char *m_root_;
    int m_log_write;
    int m_close_log;

    int pipe_fd_[2];
    int listen_fd_, epoll_fd_;
    http_conn* users;

    connection_pool *m_connPool;
    string m_user;
    string m_passWord;
    string m_databaseName;
    int m_sql_num;

    thread_pool<http_conn>*m_pool_;
    int m_thread_num;
    epoll_event events_[MAX_EVENT_NUMBER];

    Utils utils;
    client_data *users_timer;
    int user_count = 0;

};

#endif // SERVER_H