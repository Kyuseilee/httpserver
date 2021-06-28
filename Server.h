/*
 * @Author: rosonlee 
 * @Date: 2021-06-17 21:46:33 
 * @Last Modified by: rosonlee
 * @Last Modified time: 2021-06-28 19:23:30
 */

#ifndef SERVER_H
#define SERVER_H

#include "config.h"

const int BUFSIZ = 1024

class Server{
public:
    Server();
    Server(); //提供另一种重载
    ~Server();

public:
    void Init();
    void Listen();
    void Loop();

    void HandleRead();
    void HandleWrite();
    void HandleConnect();

private:
    int port_;
    int host_;


    struct sockaddr_in server_addr_, client_addr_;
    socklen_t listen_len_;
    
    int ret_;
    int listen_fd_;
    int epoll_fd_;
    struct epoll_event events_[MAX_NUMBER];
};

#endif // SERVER_H