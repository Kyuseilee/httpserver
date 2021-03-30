/*
 * @Author: rosonlee 
 * @Date: 2021-03-30 13:09:49 
 * @Last Modified by: rosonlee
 * @Last Modified time: 2021-03-30 13:27:08
 */

#include "log.h"

using namespace std;

Log::Log(){
    m_count_ = 0;
    m_is_async_ = false;
}

Log::~Log(){
    if (m_fp_ != nullptr){
        fclose(m_fp_);
    }
}

bool Log::Init(const char* file_name, int close_log, int log_buf_size, int split_lines, int max_queue_size){
    if (max_queue_size >= 1){
        m_is_async_ = true;
        m_log_queue_ = new block_queue<string>(max_queue_size);
        pthread_t tid;
        pthread_create(&tid, nullptr, FlushLogThread, nullptr);
    }

    m_close_log_ = close_log;
    m_log_buf_size_ = log_buf_size;
    m_buf_ = new char[m_log_buf_size_];
    memset(m_buf_, '\0', m_log_buf_size_);
    m_split_lines_ = split_lines;

    time_t t = time(nullptr);
    struct tm *sys_tm = localtime(&t);
    struct tm my_tm = *sys_tm;

    const char *p = strrchr(file_name, '/');
    char log_full_name[256] = {0};

    if (p == nullptr){
        snprintf(log_full_name, 255, "%d_%02d_%02d_%s", my_tm.tm_year + 1900, my_tm.tm_mon + 1, my_tm.tm_mday, file_name);
    }
    else{
        strcpy(log_name, p+1);
        strncpy(dir_name, file_name, p - file_name + 1);
        snprintf(log_full_name, 255, "%d_%02d_%02d_%s", my_tm.tm_year + 1900, my_tm.tm_mon + 1, my_tm.tm_mday, log_name);
    }

    m_today_ = my_tm.tm_mday;

    m_fp_ = fopen(log_full_name, "a");

    if (m_fp_ == nullptr)
        return false;

    return true;
}

void Log::WriteLog(int level, const char* format, ...){
    struct timeval now = {0, 0};
    gettimeofday(&now, nullptr);
    time_t t = now.tv_sec;
    struct tm *sys_tm = localtime(&t);
    struct tm my_tm = *sys_tm;
    char s[16] = {0};

    switch (level){
        case 0:
            strcpy(s, "[debug]:");
            break;
        case 1:
            strcpy(s, "[info]:");
            break;
        case 2:
            strcpy(s, "[warn]:");
            break;
        case 3:
            strcpy(s, "[errno]:");
            break;
        default:
            strcpy(s, "[info]");
            break;
    }

    m_mutex_.Lock();
    m_count_++;

    if (m_today_ != my_tm.tm_mday || m_count_ % m_split_lines_ == 0){
        char new_log[256] = {0};
        fflush(m_fp_);
        fclose(m_fp_);
        char tail[16] = {0};

        snprintf(tail, 16, "%d_%02d_%02d_", my_tm.tm_year + 1900, my_tm.tm_mon + 1, my_tm.tm_mday);

        if ( m_today_ != my_tm.tm_mday){
            snprintf(new_log, 255, "%s%s%s", dir_name, tail, log_name);
            m_today_ = my_tm.tm_mday;
            m_count_ = 0;
        }

        else{
            snprintf(new_log, 255, "%s%s%s", dir_name, tail, log_name, m_count_ / m_split_lines_);
        }
        m_fp_ = fopen(new_log, "a");
    }
    m_mutex_.Unlock();

    va_list valst;
    va_start(valst, format);

    string log_str;
    m_mutex_.Lock();

    int n = snprintf(m_buf_, 48, "%d-%02d-%02d %02d:%02d:%02d.%06ld %s ",
                    my_tm.tm_year + 1900, my_tm.tm_mon + 1, my_tm.tm_mday,
                    my_tm.tm_hour, my_tm.tm_min, my_tm.tm_sec, now.tv_usec, s);
    int m = vsnprintf(m_buf_ + n, m_log_buf_size_ - 1, format, valst);

    m_buf_[n + m] = '\n';
    m_buf_[n + m + 1] = '\0';
    log_str = m_buf_;

    m_mutex_.Unlock();

    if (m_is_async_ && !m_log_queue_->Full()){
        m_log_queue_->Push(log_str);
    }
    else{
        m_mutex_.Lock();
        fputs(log_str.c_str(), m_fp_);
        m_mutex_.Unlock();
    }

    va_end(valst);

}

void Log::Flush(void){
    m_mutex_.Lock();
    fflush(m_fp_);
    m_mutex_.Unlock();
}
