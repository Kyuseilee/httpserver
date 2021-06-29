/*
 * @Author: rosonlee 
 * @Date: 2021-06-17 22:00:28 
 * @Last Modified by:   rosonlee 
 * @Last Modified time: 2021-06-17 22:00:28 
 */


#include "Server.h"

using namespace std;

Server::Server(){

}

Server::~Server(){

}

void Server::Init(){
    //TODO \
    Init port_;

}
void Server::Listen(){
    listen_fd_ = socket(PF_INET, SOCK_STREAM, 0);

    bzero(&server_addr_, sizeof(server_addr_));
    server_addr_.sin_family = AF_INET;
    server_addr_.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr_.sin_port = port_; 
    
    ret_ = bind(listen_fd_, (struct sockaddr*)&server_addr_, sizeof(server_addr_));

    ret_ = listen(listen_fd_, 5);
}

void Server::HandleConnect(){


}

void Server::Loop(){
    while (1){
        int number = epoll_wait(epoll_fd_, &events_, MAX_NUMBER, -1)
        if (number < 0 && errno != EINTR){
        }
        for (int i = 0; i < number; ++i){
            int socket_fd = events_[i].data.fd
            uint32_t events = events_[i].events
            if (socket_fd == listen_fd_){
                HandleConnect(events_[i].fd)
            }
            else if (events_ & (EPOLLHUP | EPOLLRDHUP | EPOLLERR)){
                //TODO Break Connection
            }
            else if (events_ & EPOLLIN){
                //TODO read events
            }
            else if (events_ & EPOLLOUT){
                //TODO write events
            }
            else{
                //写日志
            }
        }
    }
}