/*
 * @Author: rosonlee 
 * @Date: 2021-03-22 19:51:32 
 * @Last Modified by:   rosonlee 
 * @Last Modified time: 2021-03-22 19:51:32 
 */

#include "server.h"

Server::Server(){

}

Server::~Server(){

}

void Server::Init(){
    //init paramater....
}
int Server::SetNonBlocking(int fd){
    int old_option = fcntl(fd, F_GETFL);
    int new_option = old_option | O_NONBLOCK;
    fcntl(fd, F_SETFL, new_option);
    return old_option;
}
void Server::AddFd(int epollfd, int fd){
    epoll_event event;
    event.data.fd = fd;
    epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event);
    SetNonBlocking(fd);
}

bool Server::Connect2Client(){
    struct sockaddr_in client_address;
    socklen_t client_length = sizeof(client_address);
    int connfd = accept(listen_fd_, (struct sockaddr*)&client_address, &client_length);
    if (connfd == -1){
        //write log info
        return false;
    }
    
    AddFd(epoll_fd_, connfd);
    return true;
}
void Server::HandleRead(int fd){
    //
    while(1){
        
    }
}
void Server::Listen(){

    char buf[] = "hello Server!\n";

    listen_fd_ = socket(PF_INET, SOCK_STREAM, 0);//Whether PF or AF
    assert(listen_fd_ >= 0);

    int ret = 0;
    struct sockaddr_in server_address;
    bzero(&server_address, sizeof(server_address));
    server_address.sin_family = PF_INET;
    server_address.sin_port = htons(port_);
    server_address.sin_addr.s_addr = htonl(INADDR_ANY);

    int flag = 1;
    setsockopt(listen_fd_, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag));//端口复用
    ret = bind(listen_fd_, (struct sockaddr*)&server_address, sizeof(server_address));
    assert(ret >= 0);
    ret = listen(listen_fd_, 5);
    assert(ret >= 0);

    // epoll_event events_[MAX_EVENT_NUMBER];
    epoll_fd_ = epoll_create(5);//number is no use, try man epoll_create for more information.
    assert(epoll_fd_ != -1);
    AddFd(epoll_fd_, listen_fd_);


}
void Server::Loop(){
    while (1){
        int number = epoll_wait(epoll_fd_, events_, MAX_EVENT_NUMBER, -1);
        if (number < 0 && errno != EINTR){
            cout << "Unexpected error!\n";
            //write info to log
            break;
        }

        for (int i = 0; i < number; i++){
            int sockfd = events_[i].data.fd;
            if (sockfd == listen_fd_){//Create client connection
                bool flag = Connect2Client();
                if(flag == false){
                    //write info to log;
                }
            }
            // else if (events_[i].events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR)){

            // }
            // else if (sockfd == m_pipefd[0] && events_[i].events & EPOLLIN){

            // }
            else if (events_[i].events & EPOLLIN){
                HandleRead(sockfd);
            }
            else if (events_[i].events & EPOLLOUT){
                HandleWrite(sockfd);
            }
        }
    }
    close(listen_fd_);
}


