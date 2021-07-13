/*
 * @Author: rosonlee 
 * @Date: 2021-07-07 20:15:20 
 * @Last Modified by: rosonlee
 * @Last Modified time: 2021-07-07 20:32:04
 */

#ifndef SQLCONNRAII_H
#define SQLCONNRAII_H

#include "sqlconnpool.h"

class SqlConnRAII
{
private:
    /* data */
public:
    SqlConnRAII(MYSQL** sql, SqlConnPool *connpool){
        assert(connpool);
        *sql = connpool->GetConn();
        sql_ = *sql;
        connpool_ = connpool;
    }
    ~SqlConnRAII(){
        if (sql_){
            connpool_->FreeConn(sql_);
        }
    }

private:
    MYSQL *sql_;
    SqlConnPool* connpool_;
};



#endif // SQLCONNRAII_H