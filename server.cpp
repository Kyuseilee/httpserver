/*
 * @Author: rosonlee 
 * @Date: 2021-03-22 19:51:32 
 * @Last Modified by: rosonlee
 * @Last Modified time: 2021-03-29 21:34:44
 */

#include "server.h"
#include "assert.h"


void Server::Timer(int conn_fd, struct sockaddr_in client_address){
    users[conn_fd].Init(conn_fd, client_address);

    users_timer[conn_fd].address_ = client_address;
    users_timer[conn_fd].sockfd_  = conn_fd;
    util_timer *timer = new util_timer;
    timer->user_data_ = &users_timer[conn_fd];
    timer->CbFunc = CbFunc;
    time_t cur = time(nullptr);
    timer->expire_ = cur + 3 * TIMESLOT;
    users_timer[conn_fd].timer_ = timer;
    utils.m_timer_lst_.AddTimer(timer);
}

void Server::AdjustTimer(util_timer *timer){
    time_t cur = time(nullptr);
    timer->expire_ = cur + 3 * TIMESLOT;
    utils.m_timer_lst_.AdjustTimer(timer);
    //Write to Log;
}

Server::Server(){
    users_timer = new client_data[MAX_FD];
    users = new http_conn[MAX_FD];
}

Server::~Server(){

}

void Server::Init(){
    port_ = 9006;
    InitThreadPool();
    //init paramater....
}

void Server::InitThreadPool(){
    m_pool_ = new thread_pool<http_conn>;
}

bool Server::HandleConnect(){
    struct sockaddr_in client_address;
    socklen_t client_length = sizeof(client_address);
    int connfd = accept(listen_fd_, (struct sockaddr*)&client_address, &client_length);
    if (connfd < 0){
        //write log info
        return false;
    }
    if (http_conn::m_user_count_ >= MAX_FD){
        utils.ShowError(connfd, "Internal server busy");
        //Write to Log;
        return false;
    }
    printf("Got connection from %d\n", connfd);
    Timer(connfd, client_address);
    return true;
}

void Server::HandleTimer(util_timer *timer, int fd){
    timer->CbFunc(&users_timer[fd]);
    if (timer){
        printf("Now %d client exit....\n", fd);
        utils.m_timer_lst_.DelTimer(timer);
    }

    //Write To Log;
}

void Server::HandleRead(int fd){
    util_timer *timer = users_timer[fd].timer_;
    if (users[fd].Read()){
        //Write to Log;

        m_pool_->Append(users + fd);
        if (timer)
            AdjustTimer(timer);
    }
    else{
        HandleTimer(timer, fd);
    }
}

void Server::HandleWrite(int fd){
    util_timer *timer = users_timer[fd].timer_;
    if (users[fd].Write()){
        //Write to Log;
        if (timer)
            AdjustTimer(timer);
    }
    else{
        HandleTimer(timer, fd);
    }
}

bool Server::HandleSignal(bool &timeout, bool &stop_server){
    int ret = 0;
    int sig;
    char signals[1024];
    ret = recv(pipe_fd_[0], signals, sizeof(signals), 0);
    if (ret == -1){
        return false;
    }
    else if (ret == 0){
        return false;
    }
    else{
        for (int i = 0; i < ret; i++){
            switch (signals[i]){
                case SIGALRM:{
                    printf("Now Here is Alarm...\n");
                    timeout = true;
                    break;
                }
                case SIGTERM:{
                    stop_server = true;
                    break;
                }
            }
        }
    }
    return true;
}
void Server::Listen(){
    listen_fd_ = socket(PF_INET, SOCK_STREAM, 0);//Whether PF or AF
    assert(listen_fd_ >= 0);

    struct linger tmp = {1, 0};
    setsockopt(listen_fd_, SOL_SOCKET, SO_LINGER, &tmp, sizeof(tmp));

    int ret = 0;
    struct sockaddr_in server_address;
    bzero(&server_address, sizeof(server_address));
    server_address.sin_family = PF_INET;
    server_address.sin_port = htons(port_);
    server_address.sin_addr.s_addr = htonl(INADDR_ANY);

    int flag = 1;
    setsockopt(listen_fd_, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag));
    ret = bind(listen_fd_, (struct sockaddr*)&server_address, sizeof(server_address));
    assert(ret >= 0);
    ret = listen(listen_fd_, 5);
    assert(ret >= 0);

    utils.Init(TIMESLOT);

    // epoll_event events_[MAX_EVENT_NUMBER];
    epoll_event events_[MAX_EVENT_NUMBER];
    epoll_fd_ = epoll_create(5);
    assert(epoll_fd_ != -1);

    utils.AddFd(epoll_fd_, listen_fd_, false);
    http_conn::m_epoll_fd_ = epoll_fd_;

    ret = socketpair(PF_UNIX, SOCK_STREAM, 0, pipe_fd_);
    assert(ret != -1);
    utils.SetNonBlocking(pipe_fd_[1]);
    utils.AddFd(epoll_fd_, pipe_fd_[0], false);

    utils.AddSig(SIGPIPE, SIG_IGN);
    utils.AddSig(SIGALRM, utils.SigHandler, false);
    utils.AddSig(SIGTERM, utils.SigHandler, false);

    alarm(TIMESLOT);
    
    Utils::u_pipe_fd_ = pipe_fd_;
    Utils::u_epoll_fd_ = epoll_fd_;
}

void Server::Loop(){
    bool timeout = false;
    bool stop_server = false;
    while (!timeout){
        int number = epoll_wait(epoll_fd_, events_, MAX_EVENT_NUMBER, -1);
        if (number < 0 && errno != EINTR){
            cout << "Unexpected error!\n";
            break;
        }
        for (int i = 0; i < number; i++){
            int sockfd = events_[i].data.fd;

            if (sockfd == listen_fd_){
                bool flag = HandleConnect();
                if (!flag)
                    continue;
            }

            else if (events_[i].events & (EPOLLHUP | EPOLLRDHUP | EPOLLERR)){
                util_timer *timer = users_timer[sockfd].timer_;
                HandleTimer(timer, sockfd);
                // users[sockfd].CloseConn();
            }
            else if ((sockfd == pipe_fd_[0]) && (events_[i].events & EPOLLIN)){
                bool flag = HandleSignal(timeout, stop_server);
                if (!flag){
                    //Write To log;
                }
            }
            else if (events_[i].events & EPOLLIN){
                HandleRead(sockfd);
            }

            else if (events_[i].events & EPOLLOUT){
                HandleWrite(sockfd);

            }
        }
        if (timeout){
            utils.TimerHandler();
            //Write to log;
            timeout = false;
        }
    }
    close(epoll_fd_);
    close(listen_fd_);
    delete []users;
    delete m_pool_;
}


