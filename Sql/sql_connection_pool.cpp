/*
 * @Author: rosonlee 
 * @Date: 2021-03-30 16:58:55 
 * @Last Modified by: rosonlee
 * @Last Modified time: 2021-03-31 10:38:21
 */

#include "sql_connection_pool.h"

using namespace std;

connection_pool::connection_pool(){
    m_CurConn_ = 0;
    m_FreeConn_ = 0;
}

connection_pool *connection_pool::GetInstance(){
    static connection_pool connPool;
    return &connPool;
}

void connection_pool::Init(string url, string User, string PassWord, string DBName, int Port, int MaxConn, int close_log){
    m_url = url;
    m_Port = Port;
    m_User = User;
    m_PassWord = PassWord;
    m_DatabaseName = DBName;
    m_close_log = close_log;

    for (int i = 0; i < MaxConn; i++){
        MYSQL *con = nullptr;
        con = mysql_init(con);
        
        if (con == nullptr){
            printf("no connn....\n");
            LOG_ERROR("MySQL Error");
            exit(1);
        }
        con = mysql_real_connect(con, url.c_str(), User.c_str(), PassWord.c_str(), DBName.c_str(), Port, nullptr, 0);

        if (con == nullptr){
            printf("No current database.....\n");
            LOG_ERROR("MySQL Error");
            exit(1);
        }
        connList_.push_back(con);
        ++m_FreeConn_;
    }
    reserve_ = sem(m_FreeConn_);

    m_MaxConn_ = m_FreeConn_;
}

MYSQL *connection_pool::GetConnection(){
    MYSQL *con = nullptr;
    if (connList_.size() == 0){
        return nullptr;
    }
    reserve_.Wait();
    lock.Lock();
    con = connList_.front();
    connList_.pop_front();

    --m_FreeConn_;
    ++m_CurConn_;

    lock.Unlock();

    return con;
}

bool connection_pool::ReleaseConnection(MYSQL *con){
    if (con == nullptr){
        return false;
    }

    lock.Lock();

    connList_.push_back(con);
    ++m_FreeConn_;
    --m_CurConn_;

    lock.Unlock();

    reserve_.Post();
    return true;
}
//销毁数据库连接池
void connection_pool::DestroyPool()
{

	lock.Lock();
	if (connList_.size() > 0)
	{
		list<MYSQL *>::iterator it;
		for (it = connList_.begin(); it != connList_.end(); ++it)
		{
			MYSQL *con = *it;
			mysql_close(con);
		}
		m_CurConn_ = 0;
		m_FreeConn_ = 0;
		connList_.clear();
	}

	lock.Unlock();
}

//当前空闲的连接数
int connection_pool::GetFreeConn()
{
	return this->m_FreeConn_;
}

connection_pool::~connection_pool()
{
	DestroyPool();
}

connectionRAII::connectionRAII(MYSQL **SQL, connection_pool *connPool){
	*SQL = connPool->GetConnection();
	
	conRAII = *SQL;
	poolRAII = connPool;
}

connectionRAII::~connectionRAII(){
	poolRAII->ReleaseConnection(conRAII);
}
