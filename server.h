/*
 * @Author: rosonlee 
 * @Date: 2021-03-22 19:51:41 
 * @Last Modified by:   rosonlee 
 * @Last Modified time: 2021-03-22 19:51:41 
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

#define MAX_FD  65535
#define MAX_EVENT_NUMBER 10000
#define BUFSIZE 1024

using namespace std;

class Server{
public:
    Server();
    ~Server();
    void Init();
    void Listen();
    void Loop();
    void AddFd(int epollfd, int fd);
    int SetNonBlocking(int fd);
    bool Connect2Client();
    void HandleWrite(int fd);
    void HandleRead(int fd);

private:
    int listen_fd_, epoll_fd_;
    int port_;
    epoll_event events_[MAX_EVENT_NUMBER];


};

#endif // SERVER_H