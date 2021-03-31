/*
 * @Author: rosonlee 
 * @Date: 2021-03-22 19:52:02 
 * @Last Modified by: rosonlee
 * @Last Modified time: 2021-03-31 12:23:26
 */

#include "http_conn.h"

locker m_lock;
map<string, string>users;
void http_conn::InitMySQLResult(connection_pool *connPool){
    mysql = NULL;
    
    connectionRAII mysqlcon(&mysql, connPool);

    //在user表中检索username，passwd数据，浏览器端输入
    if (mysql_query(mysql, "SELECT username,passwd FROM user"))
    {
        LOG_ERROR("SELECT error:%s\n", mysql_error(mysql));
    }

    //从表中检索完整的结果集
    MYSQL_RES *result = mysql_store_result(mysql);

    //返回结果集中的列数
    int num_fields = mysql_num_fields(result);

    //返回所有字段结构的数组
    MYSQL_FIELD *fields = mysql_fetch_fields(result);

    //从结果集中获取下一行，将对应的用户名和密码，存入map中
    while (MYSQL_ROW row = mysql_fetch_row(result))
    {
        string temp1(row[0]);
        string temp2(row[1]);
        users[temp1] = temp2;
    }
}

int SetNonBlocking( int fd){
    int old_option = fcntl(fd, F_GETFL);
    int new_option = old_option | O_NONBLOCK;
    fcntl(fd, F_SETFL, new_option);
    return old_option;
}
void AddFd( int epoll_fd, int fd, bool one_shot){
    epoll_event event;
    event.data.fd = fd;
    event.events = EPOLLIN |  EPOLLET | EPOLLRDHUP;
    if(one_shot){
        event.events |= EPOLLONESHOT;
    }
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &event);
    SetNonBlocking(fd);
}

int RemoveFd( int epoll_fd, int fd){
    epoll_ctl( epoll_fd, EPOLL_CTL_DEL, fd, 0);
    close(fd);
}
const char* ok_200_title = "OK";
const char* error_400_title = "Bad Request";
const char* error_400_form = "Your request has bad syntax or is inherently impossible to satisfy.\n";
const char* error_403_title = "Forbidden";
const char* error_403_form = "You do not have permission to get file form this server.\n";
const char* error_404_title = "Not Found";
const char* error_404_form = "The requested file was not found on this server.\n";
const char* error_500_title = "Internal Error";
const char* error_500_form = "There was an unusual problem serving the requested file.\n";
const char* doc_root = "./root";


void ModFd( int epoll_fd, int fd, int ev){
    epoll_event event;
    event.data.fd = fd;
    event.events = ev | EPOLLET | EPOLLONESHOT | EPOLLRDHUP;
    epoll_ctl(epoll_fd, EPOLL_CTL_MOD, fd, &event);
}

int http_conn::m_user_count_ = 0;
int http_conn::m_epoll_fd_ = -1;

void http_conn::CloseConn(bool real_close){
    if (real_close && (m_sockfd_ != -1)){
        RemoveFd(m_epoll_fd_, m_sockfd_);
        m_sockfd_ = -1;
        m_user_count_--;
    }
}
void http_conn::Init(int sockfd, const sockaddr_in &addr, int close_log, string user, string passwd, string sqlname){
    m_sockfd_ = sockfd;
    m_address_ = addr;
    AddFd(m_epoll_fd_, m_sockfd_, true);
    m_user_count_++;

    m_close_log = close_log;
    strcpy(sql_user, user.c_str());
    strcpy(sql_passwd, passwd.c_str());
    strcpy(sql_name, sqlname.c_str());

    // int reuse = 1;
    // setsockopt(m_sockfd_, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
    __Init();
}

void http_conn::__Init(){
    mysql = nullptr;
    bytes_to_send = 0;
    bytes_have_send = 0;
    
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

    cgi = 0;
    m_state = 0;
    timer_flag = 0;

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
        else if(strcasecmp(method, "POST") == 0){
            m_method_ = POST;
            cgi = 1;
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
        if (strcasecmp(m_version_, "HTTP/1.1") != 0 && strcasecmp(m_version_, "HTTP/1.0") != 0)
            return BAD_REQUEST;
        if (strncasecmp(m_url_, "http://", 7) == 0){
            m_url_ += 7;
            m_url_ = strchr(m_url_, '/');
        }
        if (strncasecmp(m_url_, "https://", 8) == 0){
            m_url_ += 8;
            m_url_ = strchr(m_url_, '/');
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
    else if (strncasecmp(text, "Host:", 5) == 0){
        text += 5;
        text += strspn(text, " \t");
        m_host_ = text;
    }
    else{
        LOG_INFO("oop!unknow header: %s", text);
    }
    return NO_REQUEST;
}

http_conn::HTTP_CODE http_conn::__ParseContent(char* text){
    if (m_read_idx_ >= (m_content_length_ + m_checked_idx_)){
        text[m_content_length_] = '\0';
        m_string_ = text;
        return GET_REQUEST;
    }
    return NO_REQUEST;
}

http_conn::HTTP_CODE http_conn::__ProcessRead(){
    LINE_STATUS line_status = LINE_OK;
    HTTP_CODE ret = NO_REQUEST;
    char* text = 0;

    while (( m_check_state_ == CHECK_STATE_CONTENT && line_status == LINE_OK) || ((line_status = __ParseLine()) == LINE_OK)){
        text = __GetLine();
        m_start_line_ = m_checked_idx_;
        LOG_INFO("%s", text)
        switch (m_check_state_){
            case CHECK_STATE_REQUESTLINE:{//Init下的初始状态
                ret = __ParseRequestLine(text);
                if (ret == BAD_REQUEST){
                    return BAD_REQUEST;
                }
                break;
            }
            case CHECK_STATE_HEADER:{
                ret = __ParseHeaders(text);
                if (ret == BAD_REQUEST){
                    return BAD_REQUEST;
                }
                else if (ret == GET_REQUEST){
                    return __DoRequest();
                }
                break;
            }
            case CHECK_STATE_CONTENT:{
                ret = __ParseContent(text);
                if (ret == GET_REQUEST){
                    return __DoRequest();
                }
                line_status = LINE_OPEN;
                break;
            }
            default:
                return INTERNAL_ERROR;
        }
    }
    return NO_REQUEST;
}

http_conn::HTTP_CODE http_conn::__DoRequest(){
    doc_root = "root";
    strcpy(m_real_file_, doc_root);
    int len = strlen(doc_root);
    
    // printf("%s\n", m_url_);
    const char *p = strrchr(m_url_, '/');

    if (cgi == 1 && (*(p+1) == '2' || *(p+1) == '3')){
        char flag = m_url_[1];

        char *m_url_real = (char *)malloc(sizeof(char) * 200);
        strcpy(m_url_real, "/");
        strcat(m_url_real, m_url_+2);
        strncpy(m_real_file_ + len, m_url_real, FILENAME_LEN - len - 1);
        free(m_url_real);

        //将用户名和密码提取出来
        //user=123&passwd=123
        char name[100], password[100];
        int i;
        for (i = 5; m_string_[i] != '&'; ++i)
            name[i - 5] = m_string_[i];
        name[i - 5] = '\0';

        int j = 0;
        for (i = i + 10; m_string_[i] != '\0'; ++i, ++j)
            password[j] = m_string_[i];
        password[j] = '\0';
        
        if (*(p + 1) == '3')
        {
            //如果是注册，先检测数据库中是否有重名的
            //没有重名的，进行增加数据
            char *sql_insert = (char *)malloc(sizeof(char) * 200);
            strcpy(sql_insert, "INSERT INTO user(username, passwd) VALUES(");
            strcat(sql_insert, "'");
            strcat(sql_insert, name);
            strcat(sql_insert, "', '");
            strcat(sql_insert, password);
            strcat(sql_insert, "')");
            printf("things to insert : %s\n", sql_insert);
            if (users.find(name) == users.end())
            {
                m_lock.Lock();
                int res = mysql_real_query(mysql, sql_insert, strlen(sql_insert));
                users.insert(pair<string, string>(name, password));
                m_lock.Unlock();

                if (!res)
                    strcpy(m_url_, "/log.html");
                else
                    strcpy(m_url_, "/registerError.html");
            }
            else
                strcpy(m_url_, "/registerError.html");
        }
        //如果是登录，直接判断
        //若浏览器端输入的用户名和密码在表中可以查找到，返回1，否则返回0
        else if (*(p + 1) == '2')
        {
            if (users.find(name) != users.end() && users[name] == password)
                strcpy(m_url_, "/welcome.html");
            else
                strcpy(m_url_, "/logError.html");
        }
    }
    
    if (*(p+1) == '0'){
        char *m_url_real = (char *)malloc(sizeof(char)*200);
        strcpy(m_url_real, "/register.html");
        strncpy(m_real_file_ + len, m_url_real, strlen(m_url_real));

        free(m_url_real);
    }

    else if(*(p+1) == '1'){
        char *m_url_real = (char *)malloc(sizeof(char) * 200);
        strcpy(m_url_real, "/log.html");
        strncpy(m_real_file_ + len, m_url_real, strlen(m_url_real));

        free(m_url_real);
    }
    else{
        strncpy(m_real_file_ + len, m_url_, FILENAME_LEN - len - 1);
    }
    if (stat(m_real_file_, &m_file_stat_) < 0){
        perror("file_error:");
        return NO_RESOURCE;
    }
    if (S_ISDIR(m_file_stat_.st_mode)){
        return BAD_REQUEST;
    }

    int fd = open(m_real_file_, O_RDONLY);
    m_file_address_ = (char*)mmap(0, m_file_stat_.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    close(fd);
    return FILE_REQUEST;
}

void http_conn::__Unmap(){
    if (m_file_address_){
        munmap(m_file_address_, m_file_stat_.st_size);
        m_file_address_ = 0;
    }
}

bool http_conn::Write(){
    int temp = 0;
    if (bytes_to_send == 0){
        ModFd(m_epoll_fd_, m_sockfd_, EPOLLIN);
        __Init();
        return true;
    }
    while(1){
        temp = writev(m_sockfd_, m_iv_, m_iv_count_);
        if (temp <= -1){
            if (errno == EAGAIN){
                ModFd(m_epoll_fd_, m_sockfd_, EPOLLOUT);
                return true;
            }
            __Unmap();
            return false;
        }
        bytes_have_send += temp;
        bytes_to_send -= temp;
        // if( bytes_to_send <= bytes_have_send){
        //     __Unmap();
        //     if(m_linger_){
        //         __Init();
        //         ModFd(m_epoll_fd_, m_sockfd_, EPOLLIN);
        //         return true;
        //     }
        //     else{
        //         ModFd(m_epoll_fd_, m_sockfd_, EPOLLIN);
        //         return false;
        //     }
        // }
        if (bytes_have_send >= m_iv_[0].iov_len)
        {
            m_iv_[0].iov_len = 0;
            m_iv_[1].iov_base = m_file_address_ + (bytes_have_send - m_write_idx_);
            m_iv_[1].iov_len = bytes_to_send;
        }
        else
        {
            m_iv_[0].iov_base = m_write_buf_ + bytes_have_send;
            m_iv_[0].iov_len = m_iv_[0].iov_len - bytes_have_send;
        }

        if (bytes_to_send <= 0)
        {
            __Unmap();
            ModFd(m_epoll_fd_, m_sockfd_, EPOLLIN);

            if (m_linger_)
            {
                __Init();
                return true;
            }
            else
            {
                return false;
            }
        }

    }
}

bool http_conn::__AddResponse(const char* format, ...){
    if (m_write_idx_ >= WRITE_BUFFER_SIZE){
        return false;
    }
    va_list arg_list;
    va_start(arg_list, format );

    int len = vsnprintf(m_write_buf_ + m_write_idx_, WRITE_BUFFER_SIZE - 1 - m_write_idx_, format, arg_list);

    if (len >= (WRITE_BUFFER_SIZE - 1 - m_write_idx_)){
        return false;
    }

    m_write_idx_ += len;
    va_end(arg_list);
    LOG_INFO("request:%s", m_write_buf_);
    return true;
}

bool http_conn::__AddStatusLine(int status, const char* title){
    return __AddResponse("%s %d %s\r\n", "HTTP/1.1/", status, title);
}

bool http_conn::__AddHeaders(int content_len){
    __AddContentLength(content_len);
    __AddLinger();
    __AddBlankLine();
}

bool http_conn::__AddContentLength(int content_len){
    return __AddResponse("Content-Length: %d\r\n", content_len);
}

bool http_conn::__AddLinger(){
    return __AddResponse("Connection: %s\r\n", (m_linger_ == true) ? "keep-alive" : "close");
}

bool http_conn::__AddBlankLine(){
    return __AddResponse("%s", "\r\n");
}

bool http_conn::__AddContent(const char* content){
    return __AddResponse("%s", content);
}

bool http_conn::__ProcessWrite(HTTP_CODE ret){
    switch ( ret ){
        case INTERNAL_ERROR:{
            __AddStatusLine(500, error_500_title);
            __AddHeaders(strlen(error_500_form));
            if (!__AddContent(error_500_form)){
                return false;
            }
            break;
        }
        case BAD_REQUEST:{
            __AddStatusLine(400, error_400_title);
            __AddHeaders(strlen(error_400_form));
            if (!__AddContent(error_400_form)){
                return false;
            }
            break;
        }
        case NO_RESOURCE:{
            __AddStatusLine(404, error_404_title);
            __AddHeaders(strlen(error_404_form));
            if (!__AddContent(error_404_form)){
                return false;
            }
            break;
        }
        case FORBIDDEN_REQUEST:{
            __AddStatusLine(403, error_403_title);
            __AddHeaders(strlen(error_403_form));
            if(!__AddContent(error_403_form)){
                return false;
            }
            break;
        }
        case FILE_REQUEST:{
            // printf("Get resource!\n\n");
            __AddStatusLine(200, ok_200_title);
            if (m_file_stat_.st_size != 0){
                __AddHeaders(m_file_stat_.st_size);
                m_iv_[0].iov_base = m_write_buf_;
                m_iv_[0].iov_len = m_write_idx_;
                m_iv_[1].iov_base = m_file_address_;
                m_iv_[1].iov_len = m_file_stat_.st_size;
                m_iv_count_ = 2;
                bytes_to_send = m_write_idx_ + m_file_stat_.st_size;
                return true;
            }
            else{
                const char* ok_string = "<html><body></body><html>";
                __AddHeaders(strlen(ok_string));
                if(!__AddContent(ok_string)){
                    return false;
                }
            }
        }
        default:
            return false;
    }
    m_iv_[0].iov_base = m_write_buf_;
    m_iv_[0].iov_len = m_write_idx_;
    m_iv_count_ = 1;
    return true;
}

void http_conn::Process(){
    HTTP_CODE read_ret = __ProcessRead();
    if (read_ret == NO_REQUEST){
        ModFd(m_epoll_fd_, m_sockfd_, EPOLLIN);
        return;
    }

    bool write_ret = __ProcessWrite(read_ret);
    if (! write_ret){
        CloseConn();
    }

    ModFd(m_epoll_fd_, m_sockfd_, EPOLLOUT);
}

