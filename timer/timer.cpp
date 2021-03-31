/*
 * @Author: rosonlee 
 * @Date: 2021-03-29 19:04:55 
 * @Last Modified by: rosonlee
 * @Last Modified time: 2021-03-31 11:57:04
 */

#include "timer.h"
#include "../http/http_conn.h"

sort_timer_lst::sort_timer_lst(){
    head_ = nullptr;
    tail_ = nullptr;
}

sort_timer_lst::~sort_timer_lst(){
    util_timer *tmp = head_;
    while(tmp){
        head_ = tmp->next_;
        delete tmp;
        tmp = head_;
    }
}

void sort_timer_lst::AddTimer(util_timer *timer){
    if (!timer){
        return ;
    }
    if (!head_){
        head_ = timer;
        tail_ = timer; 
        return ;
    }
    if (timer->expire_ < head_->expire_){
        timer->next_ = head_;
        head_->prev_ = timer;
        head_ = timer;
        return;
    }

    __AddTimer(timer, head_);
}

void sort_timer_lst::AdjustTimer(util_timer *timer){
    if (!timer){
        return ;
    }

    util_timer *tmp = timer->next_;
    if (!tmp || (timer->expire_ < tmp->expire_)){
        return ;
    }

    if (timer == head_){
        head_ = head_->next_;
        head_->prev_ = nullptr;
        timer->next_ = nullptr;
        __AddTimer(timer, head_);
    }
    else{
        timer->prev_->next_ = timer->next_;
        timer->next_->prev_ = timer->prev_;
        __AddTimer(timer, timer->next_);
    }
}

void sort_timer_lst::DelTimer(util_timer *timer){
    if (!timer){
        return;
    }

    if ((timer == head_) && (timer == tail_)){
        delete timer;
        head_ = nullptr;
        tail_ = nullptr;
        return ;
    }

    if (timer == head_){
        head_ = head_->next_;
        head_->prev_ = nullptr;
        delete timer;
        return ;
    }

    if (timer == tail_){
        tail_ = tail_->prev_;
        tail_->next_ = nullptr;
        delete timer;
        return ;
    }

    timer->prev_->next_ = timer->next_;
    timer->next_->prev_ = timer->prev_;
    delete timer;
}

void sort_timer_lst::Tick(){
    if (!head_){
        return ;
    }

    time_t cur = time(nullptr);
    util_timer *tmp = head_;
    while (tmp){
        if (cur < tmp->expire_){
            break;
        }
        tmp->CbFunc(tmp->user_data_);
        head_ = tmp->next_;
        if (head_){
            head_->prev_ = nullptr;
        }
        delete tmp;
        tmp = head_;
    }
}

void sort_timer_lst::__AddTimer(util_timer *timer, util_timer *lst_head){
    util_timer *prev = lst_head;
    util_timer *tmp = prev->next_;

    while (tmp){
        if (timer->expire_ < tmp->expire_){
            prev->next_ = timer;
            timer->next_ = tmp;
            tmp->prev_ = timer;
            timer->prev_ = prev;
            break;
        }
        prev = tmp;
        tmp = tmp->next_;
    }

    if (!tmp){
        prev->next_ = timer;
        timer->prev_ = prev;
        timer->next_ = nullptr;
        tail_ = timer;
    }
}

void Utils::Init(int timeslot){
    m_TIMESLOT_ = timeslot;
}

int Utils::SetNonBlocking(int fd){
    int old_option = fcntl(fd, F_GETFL);
    int new_option = old_option | O_NONBLOCK;
    fcntl(fd, F_SETFL, new_option);
    return old_option;
}

void Utils::AddFd(int epoll_fd, int fd, bool one_shot){
    epoll_event event;
    event.data.fd = fd;
    
    event.events = EPOLLIN |  EPOLLET | EPOLLRDHUP;

    if(one_shot){
        event.events |= EPOLLONESHOT;
    }
    printf("Now we add %d fd to handle %d client...\n", epoll_fd, fd);
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &event);
    SetNonBlocking(fd);
}

void Utils::SigHandler(int sig){
    int save_errno = errno;
    int msg = sig;
    send(u_pipe_fd_[1], (char *)&msg, 1, 0);
    errno = save_errno;
}

void Utils::AddSig(int sig, void(handler)(int), bool restart){
    struct sigaction sa;
    memset(&sa, '\0', sizeof(sa));
    sa.sa_handler = handler;
    if(restart){
        sa.sa_flags |= SA_RESTART;
    }
    sigfillset(&sa.sa_mask);
    assert(sigaction(sig, &sa, NULL) != -1);
}

void Utils::TimerHandler(){
    m_timer_lst_.Tick();
    alarm(m_TIMESLOT_);
}

void Utils::ShowError(int conn_fd, const char* info){
    send(conn_fd, info, strlen(info), 0);
    close(conn_fd);
}

int *Utils::u_pipe_fd_ = 0;
int Utils::u_epoll_fd_ = 0;

class Utils;
void CbFunc(client_data *user_data){
    epoll_ctl(Utils::u_epoll_fd_, EPOLL_CTL_DEL, user_data->sockfd_, 0);
    assert(user_data);
    close(user_data->sockfd_);
    http_conn::m_user_count_--;
}

