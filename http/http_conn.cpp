/*
 * @Author: rosonlee 
 * @Date: 2021-03-22 19:52:02 
 * @Last Modified by: rosonlee
 * @Last Modified time: 2021-03-23 21:00:45
 */

#include "http_conn.h"

const char* ok_200_title = "OK";
const char* error_400_title = "Bad Request";
const char* error_400_form = "Your request has bad syntax or is inherently impossible to satisfy.\n";
const char* error_403_title = "Forbidden";
const char* error_403_form = "You do not have permission to get file form this server.\n";
const char* error_404_title = "Not Found";
const char* error_404_form = "The requested file was not found on this server.\n";
const char* error_500_title = "Internal Error";
const char* error_500_form = "There was an unusual problem serving the requested file.\n";
const char* doc_root = "/var/www/html";

int SetNonBlocking( int fd){
    int old_option = fcntl(fd, F_GETFL);
    int new_option = old_option | O_NONBLOCK;
    fcntl(fd, F_SETFL, new_option);
    return old_option;
}

void AddFd( int epoll_fd, int fd, bool one_shot){
    epoll_event event;
    event.data.fd = fd;
    event.events = EPOLLIN |  EPOLLET || EPOLLRDHUP;
    if(one_shot){
        event.events |= EPOLLONESHOT;
    }
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &event);
    SetNonBlocking(fd);
}

void RemoveFd( int epoll_fd, int fd){
    epoll_ctl( epoll_fd, EPOLL_CTL_DEL, fd, 0);
    close(fd);
}

void ModFd( int epoll_fd, int fd, int ev){
    epoll_event event;
    event.data.fd = fd;
    event.events = ev | EPOLLET | EPOLLONESHOT | EPOLLRDHUP;
    epoll_ctl(epoll_fd, EPOLL_CTL_MOD, fd, &event);
}

void http_conn::CloseConn(bool real_close){
    if (real_close && (m_sockfd_ != -1)){
        RemoveFd(m_epoll_fd_, m_sockfd_);
        m_sockfd_ = -1;
        m_user_count_--;
    }
}

void http_conn::Init(int sockfd, const sockaddr_in& addr){
    m_sockfd_ = sockfd;
    m_address_ = addr;
    int reuse = 1;
    setsockopt(m_sockfd_, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
    m_user_count_++;
}

void http_conn::__Init(){
    m_check_state_ = CHECK_STATE_REQUESTLINE;
    m_linger_ = false;
    
    m_method_ = GET;
    m_url_ = 0;
    m_version_ = 0;
    m_content_length_ = 0;
    m_host_ = 0;
    m_start_line_ = 0;
    m_checked_idx_ = 0;
    m_read_idx_ = 0;
    m_write_idx_ = 0;

    memset(m_read_buf_, '\0', READ_BUFFER_SIZE);
    memset(m_write_buf_, '\0', WRITE_BUFFER_SIZE);
    memset(m_real_file_, '\0', FILENAME_LEN);
}

http_conn::LINE_STATUS http_conn::__ParseLine(){
    char temp;
    for (; m_checked_idx_ < m_read_idx_; m_checked_idx_++){
        temp = m_read_buf_[m_checked_idx_];
        if (temp == '\r'){
            if ((m_checked_idx_ + 1) == m_read_idx_)
                return LINE_OPEN;
            else if (m_read_buf_[m_checked_idx_ + 1 == '\n']){
                m_read_buf_[m_checked_idx_++] = '\0';
                m_read_buf_[m_checked_idx_++] = '\0';
                return LINE_OK;
            }
            return LINE_BAD;
        }
        else if (temp == '\n'){
            if ((m_checked_idx_ > 1 ) && (m_read_buf_[m_checked_idx_-1] == '\r')){
                m_read_buf_[m_checked_idx_-1] = '\0';
                m_read_buf_[m_checked_idx_++] = '\0';
                return LINE_OK;
            }
            return LINE_BAD;
        }
    }
    return LINE_OPEN;
}

bool http_conn::Read(){
    if (m_read_idx_ >= READ_BUFFER_SIZE){
        return false;
    }

    int bytes_read = 0;
    while(true){
        bytes_read = recv(m_sockfd_, m_read_buf_ + m_read_idx_, READ_BUFFER_SIZE - m_read_idx_, 0);
        if (bytes_read == -1){
            if (errno == EAGAIN || errno == EWOULDBLOCK)
                break;
            return false;
        }
        else if (bytes_read == 0)
            return false;
        
        m_read_idx_ += bytes_read;
    }
    return true;
}

http_conn::HTTP_CODE http_conn::__ParseRequestLine(char *text){
        m_url_ = strpbrk(text, " \t");
        if (!m_url_){
            return BAD_REQUEST;
        }
        *m_url_++ = '\0';

        char* method = text;
        if (strcasecmp(method, "GET") == 0){
            m_method_ = GET;
        }
        //more function
        else{
            return BAD_REQUEST;
        }

        m_url_ += strspn( m_url_, " \t");
        m_version_ = strpbrk( m_url_, " \t");
        if( !m_version_)
            return BAD_REQUEST;;
        *m_version_++ = '\0';
        m_version_ += strspn(m_version_, " \t");
        if (strcasecmp(m_version_, "HTTP/1.1") != 0)
            return BAD_REQUEST;
        if (strncasecmp(m_url_, "http://", 7) == 0){
            m_url_ += 7;
            m_url_ = strchr( m_url_, '/');
        }
        if (strncasecmp(m_url_, "https://", 8) == 0){
            m_url_ += 8;
            m_url_ = strchr( m_url_, '/');
        }
        if (!m_url_ || m_url_[0] != '/'){
            return BAD_REQUEST;
        }

        if (strlen(m_url_) == 1){
            strcat(m_url_, "index.html");
        }
        m_check_state_ = CHECK_STATE_HEADER;
        return NO_REQUEST;
}

http_conn::HTTP_CODE http_conn::__ParseHeaders(char* text){
    if (text[0] == '\0'){
        if( m_content_length_ != 0){
            m_check_state_ = CHECK_STATE_CONTENT;
            return NO_REQUEST;
        }
        return GET_REQUEST;
    }
    else if (strncasecmp(text, "Connection:", 11) == 0){
        text += 11;
        text += strspn(text, " \t");
        if (strcasecmp(text, "keep-alive") == 0){
            m_linger_ = true;
        }
    }
    else if (strncasecmp(text, "Content-length:", 15) == 0){
        text += 15;
        text += strspn(text, " \t");
        m_content_length_ = atol(text);
    }
    else if (strncasecmp( text, "Host:", 5) == 0){
        text += 5;
        text += strspn(text, " \t");
        m_host_ = text;
    }
    else{
        printf("oops! unknown header %s\n", text);
    }
    return NO_REQUEST;
}

http_conn::HTTP_CODE http_conn::__ParseContent(char* text){
    if (m_read_idx_ >= (m_content_length_ + m_checked_idx_)){
        text[m_content_length_] = '\0';
        return GET_REQUEST;
    }
    return NO_REQUEST;
}

http_conn::HTTP_CODE http_conn::__ProcessRead(){
    LINE_STATUS line_status = LINE_OK;
    HTTP_CODE ret = NO_REQUEST;
    char* text = 0;

    while ((( m_check_state_ == CHECK_STATE_CONTENT) && (line_status == LINE_OK)) || (line_status = __ParseLine()) == LINE_OK){
        text = __GetLine();
        m_start_line_ = m_checked_idx_;
        printf("got 1 http line: %s\n", text);
        switch (m_check_state_){
            case CHECK_STATE_REQUESTLINE:{
                ret = __ParseRequestLine(text);
                if (ret == BAD_REQUEST)
            }
        }
    }
}


