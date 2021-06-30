/*
 * @Author: rosonlee 
 * @Date: 2021-06-17 22:00:28 
 * @Last Modified by: rosonlee
 * @Last Modified time: 2021-06-30 20:37:12
 */


#include "Server.h"

using namespace std;

Server::Server(
        int port, int timeoutMs, bool openLinger) : 
        port_(port), timeoutMs_(timeoutMs), isClosed_(false),
        openLinger_(openLinger), listen_event_(EPOLLRDHUP),conn_event_(EPOLLONESHOT | EPOLLRDHUP) {

        if (!__InitSocket()) {isClosed_ = true;}

}

Server::~Server(){

}

bool Server::__InitSocket(){
    int ret;

    struct sockaddr_in server_addr_;
    if (port_ > 65535 || port_ < 1024){
        //TODO Write Into Log
        return false;
    }
    bzero(&server_addr_, sizeof(server_addr_));
    server_addr_.sin_family = AF_INET;
    server_addr_.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr_.sin_port = port_; 
    struct linger olinger = {0};
    if (openLinger_){
        olinger.l_onoff = 1;
        olinger.l_linger = 1;
    }

    listen_fd_ = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd_ < 0){
        //TODO Write Into Log
        return false;
    }    
    ret = setsockopt(listen_fd_, SOL_SOCKET, SO_LINGER, &olinger, sizeof(olinger));
    if (ret < 0){
        close(listen_fd_);
        //TODO Write Into Log
        return false;
    }

    int flag = 1;
    ret = setsockopt(listen_fd_, SOL_SOCKET, SO_REUSEADDR, (const void*)&flag, sizeof(flag));
    if (ret == -1){
        //TODO Log
        close(listen_fd_);
        return false;
    }
    ret = bind(listen_fd_, (struct sockaddr*)&server_addr_, sizeof(server_addr_));
    if (ret < 0){
        //TODO Log
        close(listen_fd_);
        return false;
    }

    ret = listen(listen_fd_, 5);
    if (ret < 0){
        //TODO Log
        close(listen_fd_);
        return false;
    }
    
    ret = epoller_->AddFd(listen_fd_, EPOLLIN | listen_event_);
    if (ret == 0){
        //TODO Log
        close(listen_fd_);
        return false;
    }
    
    __SetNonBlock(listen_fd_);
    //TODO Log Info
    return true;

}

void Server::__SetNonBlock(int fd){
    assert(fd > 0);
    return fcntl(fd, F_SETFL, fcntl(fd, F_GETFD, 0) | O_NONBLOCK);
}

void Server::__HandleListen(){

}

void Server::__HandleConnect(){

}

void Server::__HandleRead(){

}

void Server::__HandleWrite(){

}

void Server::__CloseConn(HttpConn* client){
    assert(client);
    //TODO Log
    //TODO User close;
    epoller_->DelFd(client->GetFd());
}

void Server::Loop(){
    int timeMs = -1;
    if (isClosed_){
        //TODO LOG
    }
    while (!isClosed_){
        //TODO Timer
        int number = epoller_->Wait(timeMs)
        if (number < 0 && errno != EINTR){
            //LOG
        }
        for (int i = 0; i < number; ++i){
            int socket_fd = epoller_->GetEventFd(i);
            uint32_t events = epoller_->GetEvents(i);
            if (socket_fd == listen_fd_){
                HandleListen(events_[i].fd)
            }
            else if (events_ & (EPOLLHUP | EPOLLRDHUP | EPOLLERR)){
                //TODO Break Connection
                //TODO Close fd
                CloseConn();
            }
            else if (events_ & EPOLLIN){
                //TODO read events
                HandleRead();
            }
            else if (events_ & EPOLLOUT){
                //TODO write events
                HandleWrite();
            }
            else{
                //TODO Log
            }
        }
    }
}