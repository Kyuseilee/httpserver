/*
 * @Author: rosonlee 
 * @Date: 2021-07-07 20:20:05 
 * @Last Modified by: rosonlee
 * @Last Modified time: 2021-07-07 20:30:01
 */

#include "sqlconnpool.h"

using namespace std;

SqlConnPool::SqlConnPool(){
    user_count_ = 0;
    free_count_ = 0;
}

SqlConnPool* SqlConnPool::Instance(){
    static SqlConnPool connPool;
    return &connPool;
}

void SqlConnPool::Init(const char* host ,int port, 
                const char* user, const char* pwd, const char* dbName,
                int connSize = 10){
    assert(connSize > 0);
    for (int i = 0; i < connSize; ++i){
        MYSQL* sql = nullptr;
        sql = mysql_init(sql);
        if (!sql){
            LOG_ERROR("Mysql init error!");
            assert(sql);
        }
        sql = mysql_real_connect(sql, host, user, pwd, dbName, port, nullptr, 0);
        if (!sql){
            LOG_ERROR("Mysql connect error!");
        }
        connQue_.push(sql);
    }
    MAX_CONN_ = connSize;
    sem_init(&sem_id_, 0, MAX_CONN_);
}

MYSQL* SqlConnPool::GetConn(){
    MYSQL* sql = nullptr;
    if (connQue_.empty()){
        LOG_WARN("SqlConnPool Busy!");
        return nullptr;
    }
    sem_wait(&sem_id_);
    {
        lock_guard<mutex>locker(mtx_);
        sql = connQue_.front();
        connQue_.pop();
    }
    return sql;
}

void SqlConnPool::FreeConn(MYSQL *sql){
    assert(sql);
    lock_guard<mutex>locker(mtx_);
    connQue_.push(sql);
    sem_post(&sem_id_);
}

void SqlConnPool::ClosePool(){
    lock_guard<mutex>locker(mtx_);
    while (!connQue_.empty()){
        auto item = connQue_.front();
        connQue_.pop();
        mysql_close(item);
    }
    mysql_library_end();
}

int SqlConnPool::GetFreeConnCount(){
    lock_guard<mutex>locker(mtx_);
    return connQue_.size();
}

SqlConnPool::~SqlConnPool(){
    ClosePool();
}

