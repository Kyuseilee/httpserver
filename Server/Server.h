/*
 * @Author: rosonlee 
 * @Date: 2021-06-17 21:46:33 
 * @Last Modified by: rosonlee
 * @Last Modified time: 2021-06-30 20:57:08
 */

#ifndef SERVER_H
#define SERVER_H

#include <unordered_map>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include "../config.h"
#include "epoller.h"
#include "../threadpool.h"

const int BUFSIZE = 1024;
class Server{
public:
    Server(int port, int timeoutMs, bool openLinger);
    ~Server();

public:
    void Loop();


private:
    bool __InitSocket();
    void __HandleRead();
    void __HandleWrite();
    void __HandleConnect();
    void __HandleListen();

    void __CloseConn();


    void __SetNonBlock(int fd);

private:
    int port_;
    int timeoutMs_;
    bool openLinger_;

    bool isClosed_;

    uint32_t listen_event_;
    uint32_t conn_event_;

    socklen_t listen_len_;
    
    int listen_fd_;
    int epoll_fd_;

    std::unique_ptr<Epoller>epoller_;

};

#endif // SERVER_H