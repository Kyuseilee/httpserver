/*
 * @Author: rosonlee 
 * @Date: 2021-06-30 19:07:27 
 * @Last Modified by: rosonlee
 * @Last Modified time: 2021-06-30 19:45:13
 */



#include "epoller.h"

Epoller::Epoller(int maxEvent) : epoll_fd_(epoll_create(512)), events_(maxEvent){
    assert(epoll_fd_ >= 0 && events_.size() > 0);
}

Epoller::~Epoller(){
    close(epoll_fd_);
}

bool Epoller::AddFd(int fd, uint32_t event){
    if (fd < 0) return false;
    epoll_event ev = {0};
    ev.data.fd = fd;
    ev.events = event;
    return 0 == epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, fd, &ev);
}

bool Epoller::ModFd(int fd, uint32_t event){
    if (fd < 0) return false;
    epoll_event ev = {0};
    ev.data.fd = fd;
    ev.events = event;
    return 0 == epoll_ctl(epoll_fd_, EPOLL_CTL_MOD, fd, &ev);
}

bool Epoller::DefFd(int fd){
    if (fd < 0) return false;
    epoll_event ev = {0};
    return 0 == epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, fd, &ev);
}

int Wait(int timeoutMs){
    return epoll_wait(epoll_fd_, &events_[0], static_cast<int>(events_.size()), timeoutMs);
}

int Epoller::GetEventFd(size_t i){
    assert(i < events_.size() && i >= 0);
    return events_[i].data.fd;
}

uint32_t Epoller::GetEvents(size_t i){
    assert(i < events_.size() && i >= 0);
    return events_[i].events;
}