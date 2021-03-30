/*
 * @Author: rosonlee 
 * @Date: 2021-03-30 12:02:24 
 * @Last Modified by: rosonlee
 * @Last Modified time: 2021-03-30 19:12:11
 */
#ifndef LOG_H
#define LOG_H

#include <stdio.h>
#include <iostream>
#include <string>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <stdarg.h>
#include <pthread.h>
#include "block_queue.h"

using namespace std;

class Log{
public:
    static Log *GetInstance(){
        static Log instance;
        return &instance;
    }

    static void* FlushLogThread(void *args){
        Log::GetInstance()->__AsyncWriteLog();
    }

    bool Init(const char* file_name, int close_log, int log_buf_size = 8192, int split_lines = 5000000, int max_queue_size = 0);

    void WriteLog(int level, const char* format, ...);
    void Flush(void);

private:
    Log();
    virtual ~Log();
    void *__AsyncWriteLog(){
        string single_log_;
        while (m_log_queue_->Pop(single_log_)){
            m_mutex_.Lock();
            fputs(single_log_.c_str(), m_fp_);
            m_mutex_.Unlock();
        }
    }

private:
    char dir_name[128];
    char log_name[128];
    int m_split_lines_;
    int m_log_buf_size_;
    long long m_count_;
    int m_today_;
    FILE *m_fp_;
    char *m_buf_;
    block_queue<string> *m_log_queue_;
    bool m_is_async_;
    locker m_mutex_;
    int m_close_log;
};

#define LOG_DEBUG(format, ...) if(0 == m_close_log) {Log::GetInstance()->WriteLog(0, format, ##__VA_ARGS__); Log::GetInstance()->Flush();}
#define LOG_INFO(format, ...) if(0 == m_close_log) {Log::GetInstance()->WriteLog(1, format, ##__VA_ARGS__); Log::GetInstance()->Flush();}
#define LOG_WARN(format, ...) if(0 == m_close_log) {Log::GetInstance()->WriteLog(2, format, ##__VA_ARGS__); Log::GetInstance()->Flush();}
#define LOG_ERROR(format, ...) if(0 == m_close_log) {Log::GetInstance()->WriteLog(3, format, ##__VA_ARGS__); Log::GetInstance()->Flush();}

#endif // LOG_H
