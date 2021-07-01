/*
 * @Author: rosonlee 
 * @Date: 2021-06-17 21:47:35 
 * @Last Modified by: rosonlee
 * @Last Modified time: 2021-06-30 22:46:49
 */

#ifndef HTTP_CONN_H
#define HTTP_CONN_H

#include <sys/types.h>
#include <sys/uio.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <errno.h>

#include "httprequest.h"
#include "httpresponse.h"

class HttpConn
{
private:
    int fd_;
    struct sockaddr_in addr_;

    bool isClose_;

public:
    static const char* srcDir;
    static std::atomic<int> userCount;
    

public:
    HttpConn(/* args */);
    ~HttpConn();

    void Init(int fd, const sockaddr_in& addr);

    ssize_t Read(int* saveErrno);
    ssize_t Write(int* saveErrno);

    void Close();

    int GetFd() const;
    int GetPort() const;
    const char* GetIP() const;
    sockaddr_in GetAddr() const;




};



#endif // HTTP_CONN_H