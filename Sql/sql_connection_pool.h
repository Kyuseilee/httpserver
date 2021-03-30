/*
 * @Author: rosonlee 
 * @Date: 2021-03-30 16:46:33 
 * @Last Modified by: rosonlee
 * @Last Modified time: 2021-03-30 16:59:34
 */

#ifndef SQL_CONNECTION_POOL_H
#define SQL_CONNECTION_POOL_H

#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>
#include<list>
#include<mysql/mysql.h>
#include<error.h>
#include<string.h>
#include<iostream>
#include<string>
#include "../locker/locker.h"
#include "../log/log.h"

using namespace std;

class connection_pool{
public:
    MYSQL *GetConnection();
    bool ReleaseConnection(MYSQL *conn);
    int GetFreeConn();
    void DestroyPool();

    static connection_pool *GetInstance();
    
    void Init(string url, string User, string PassWord, string DataBaseName, int Port, int MaxConn, int close_log);

private:
    connection_pool();
    ~connection_pool();

    int m_MaxConn_;
    int m_CurConn_;
    int m_FreeConn_;
    locker lock;
    list<MYSQL *>connList_;
    sem reserve_;

public:
    string m_url;
    string m_Port;
    string m_User;
    string m_PassWord;
    string m_DatabaseName;
    int m_close_log;
};

class connectionRAII{
public:
    connectionRAII(MYSQL **con, connection_pool *connPool);
    ~connectionRAII();
private:
    MYSQL *conRAII;
    connection_pool *poolRAII;
};
#endif // SQL_CONNECTION_POOL_H