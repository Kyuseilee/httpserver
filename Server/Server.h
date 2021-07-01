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
#include "../httpparse/http_conn.h"

const int BUFSIZE = 1024;
class Server{
public:
    Server(int port, int timeoutMs, bool openLinger);
    ~Server();

    void Loop();

private:
    bool __InitSocket();

    void __AddClient(int fd, sockaddr_in addr);
    void __HandleListen();

    void __HandleRead(HttpConn* fd);
    void __HandleWrite(HttpConn* fd);
    void __HandleClose(HttpConn* fd);

    void __OnRead(HttpConn* client);
    void __OnWrite(HttpConn* client);
    void __OnProcess(HttpConn* client);

private:
    static int __SetNonBlock(int fd);
    static const int MAX_FD = 65536;

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
    std::unique_ptr<ThreadPool> threadpool_;
    std::unordered_map<int, HttpConn*>user_;

};

#endif // SERVER_H