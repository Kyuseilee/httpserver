/*
 * @Author: rosonlee 
 * @Date: 2021-03-22 19:51:59 
 * @Last Modified by: rosonlee
 * @Last Modified time: 2021-03-30 19:20:29
 */
#ifndef HTTP_CONN_H
#define HTTP_CONN_H

#include<unistd.h>
#include<signal.h>
#include<sys/types.h>
#include<sys/epoll.h>
#include<fcntl.h>
#include<sys/uio.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<assert.h>
#include<sys/stat.h>
#include<string.h>
#include<pthread.h>
#include<stdio.h>
#include<stdlib.h>
#include<sys/mman.h>
#include<stdarg.h>
#include<errno.h>
#include<map>

#include "../locker/locker.h"
#include "../timer/timer.h"
#include "../Sql/sql_connection_pool.h"
#include "../log/log.h"


class http_conn{

public:
static const int FILENAME_LEN = 200;
static const int READ_BUFFER_SIZE = 2048;
static const int WRITE_BUFFER_SIZE = 1024;
enum METHOD { GET = 0, POS, HEAD, PUT, DELETE, TRACE, OPTIONS, CONNECT, PATCH};
enum CHECK_STATE { CHECK_STATE_REQUESTLINE = 0, CHECK_STATE_HEADER, CHECK_STATE_CONTENT};
enum HTTP_CODE { NO_REQUEST, GET_REQUEST, BAD_REQUEST, NO_RESOURCE, FORBIDDEN_REQUEST, FILE_REQUEST, INTERNAL_ERROR, CLOSED_CONNECTION };
enum LINE_STATUS { LINE_OK = 0, LINE_BAD, LINE_OPEN };

public:
    http_conn(){}
    ~http_conn(){}

public:
    void Init(int sockfd, const sockaddr_in &addr, int, string user, string passwd, string sqlname);
    void CloseConn( bool real_close = true);
    void Process();
    bool Read();
    bool Write();
    void InitMySQLResult(connection_pool *connPool);
    int timer_flag;
    int improv;


private:
    void __Init();
    HTTP_CODE __ProcessRead();
    bool __ProcessWrite( HTTP_CODE ret);
    
    HTTP_CODE __ParseRequestLine( char* text);
    HTTP_CODE __ParseHeaders( char* text);
    HTTP_CODE __ParseContent( char* text);
    HTTP_CODE __DoRequest();
    char* __GetLine() { return m_read_buf_ + m_start_line_; }
    LINE_STATUS __ParseLine();
    
    void __Unmap();
    bool __AddResponse( const char* format, ...);
    bool __AddContent( const char* content);
    bool __AddStatusLine( int status, const char* title);
    bool __AddHeaders( int content_length);
    bool __AddContentLength( int content_length);
    bool __AddLinger();
    bool __AddBlankLine();


public:
    static int m_epoll_fd_;
    static int m_user_count_;
    MYSQL *mysql;
    int m_state;

private:
    int m_sockfd_;
    sockaddr_in m_address_;
    char m_read_buf_[READ_BUFFER_SIZE];
    int m_read_idx_;
    int m_checked_idx_;
    int m_start_line_;
    char m_write_buf_[WRITE_BUFFER_SIZE];
    int m_write_idx_;
    
    CHECK_STATE m_check_state_;
    METHOD m_method_;

    char m_real_file_[FILENAME_LEN];
    char* m_url_;
    char* m_version_;
    char* m_host_;
    int m_content_length_;
    bool m_linger_;
    
    char* m_file_address_;
    struct stat m_file_stat_;
    struct iovec m_iv_[2];
    int m_iv_count_;

    int cgi;
    char *m_string_;
    int bytes_have_send ;
    int bytes_to_send;
    char *doc_root;

    int m_close_log;
    char sql_user[100];
    char sql_passwd[100];
    char sql_name[100];
};

#endif // HTTP_CONN_H
