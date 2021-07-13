/*
 * @Author: rosonlee 
 * @Date: 2021-07-07 20:15:37 
 * @Last Modified by: rosonlee
 * @Last Modified time: 2021-07-07 20:19:42
 */

#ifndef SQLCONNPOOL_H
#define SQLCONNPOOL_H

#include <mysql/mysql.h>
#include <string>
#include <queue>
#include <mutex>
#include <semaphore.h>
#include <thread>
#include <../log/log.h>

class SqlConnPool
{
private:
    SqlConnPool(/* args */);
    ~SqlConnPool();
public:
    static SqlConnPool *Instance();

    MYSQL* GetConn();
    void FreeConn(MYSQL* conn);
    int GetFreeConnCount();

    void Init(const char* host, int post,
              const char* user, const char* pwd,
              const char* dbName, int connSize);
    void ClosePool();

private:
    int MAX_CONN_;
    int user_count_;
    int free_count_;

    std::queue<MYSQL*> connQue_;
    std::mutex mtx_;
    sem_t sem_id_;
};

#endif // SQLCONNPOOL_H