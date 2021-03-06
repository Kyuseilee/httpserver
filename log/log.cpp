/*
 * @Author: rosonlee 
 * @Date: 2021-07-07 19:10:59 
 * @Last Modified by: rosonlee
 * @Last Modified time: 2021-07-07 19:42:33
 */

#include "log.h"

using namespace std;

Log::Log(){
    lineCount_ = 0;
    isAsync_ = false;
    writeThread_ = nullptr;
    que_ = nullptr;
    toDay_ = 0;
    fp_ = nullptr;
}

Log::~Log(){
    if (writeThread_ && writeThread_->joinable()){
        while(!que_->empty()){
            que_->flush();
        };
        que_->Close();
        writeThread_->join();
    }
    if (fp_){
        lock_guard<mutex>locker(mtx_);
        Flush();
        fclose(fp_);
    }
}

int Log::GetLevel(){
    lock_guard<mutex>locker(mtx_);
    return level_;
}

void Log::SetLevel(int level){
    lock_guard<mutex>locker(mtx_);
    level_ = level;
}

void Log::Init(int level = 1, const char* path, const char* suffix, int maxQueueSize){
    isOpen_ = true;
    level_ = level;
    if (maxQueueSize > 0){
        unique_ptr<BlockQueue<string>> newQueue(new BlockQueue<string>);
        que_ = move(newQueue);

        unique_ptr<thread> newThread(new thread(FlushLogThread));
        writeThread_ = move(newThread);
    }
    else{
        isAsync_ = false;
    }

    lineCount_ = 0;

    time_t timer = time(nullptr);
    struct tm* sysTime = localtime(&timer);
    struct tm t = *sysTime;
    path_ = path;
    suffix_ = suffix;
    char fileName[LOG_NAME_LEN] = {0};
    snprintf(fileName, LOG_NAME_LEN-1, "%s/%04d_%02d_%02d%s",
            path_, t.tm_year + 1900, t.tm_mon + 1, t.tm_mday, suffix_);
    toDay_ = t.tm_mday;
    {
        lock_guard<mutex>locker(mtx_);
        buff_.RetrieveAll();
        if (fp_){
            Flush();
            fclose(fp_);
        }

        fp_ = fopen(fileName, "a");
        if (fp_ == nullptr){
            mkdir(path_, 0777);
            fp_ = fopen(fileName, "a");
        }
        assert(fp_ != nullptr);
    }
}

void Log::Write(int level, const char* format, ...){
    struct timeval now = {0, 0};
    gettimeofday(&now, nullptr);
    time_t tSec = now.tv_sec;
    struct tm *sysTime = localtime(&tSec);
    struct tm t = *sysTime;
    va_list vaList;

    if (toDay_ != t.tm_mday || (lineCount_ && (lineCount_ % MAX_LINES == 0))){
        unique_lock<mutex>locker(mtx_);
        locker.unlock();

        char newFile[LOG_NAME_LEN];
        char tail[36] = {0};
        snprintf(tail, 36, "%04d_%02d_%02d", t.tm_year+1900, t.tm_mon + 1, t.tm_mday);

        if (toDay_ != t.tm_mday){
            snprintf(newFile, LOG_NAME_LEN - 72, "%s/%s%s", path_, tail, suffix_);
            toDay_ = t.tm_mday;
            lineCount_ = 0;
        }

        else{
            snprintf(newFile, LOG_NAME_LEN - 72, "%s/%s-%d%s", path_, tail, (lineCount_ / MAX_LINES),  suffix_);
        }
        locker.lock();
        Flush();
        fclose(fp_);
        fp_ = fopen(newFile, "a");
        assert(fp_ != nullptr);
    }
    {
        unique_lock<mutex>locker(mtx_);
        lineCount_++;
        int n = snprintf(buff_.BeginWrite(), 128, "%d-%02d-%02d %02d:%02d:%02d.%06ld ", t.tm_year+1900, t.tm_mon + 1, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec, now.tv_usec);

        buff_.HasWritten(n);
        __AppendLogLevelTitle(level);

        va_start(vaList, format);
        int m = vsnprintf(buff_.BeginWrite(), buff_.WritableBytes(), format, vaList);
        va_end(vaList);

        buff_.HasWritten(m);
        buff_.Append("\n\0", 2);

        if (isAsync_ && que_ && !que_->isfull()){
            que_->push(buff_.RetrieveAllToStr());
        }
        else{
            fputs(buff_.Peek(), fp_);
        }
        buff_.RetrieveAll();
    }
}


void Log::__AppendLogLevelTitle(int level){
    switch (level){
    case 0:
        buff_.Append("[debug]: ", 9);
        break;
    case 1:
        buff_.Append("[info] : ", 9);
        break;
    case 2:
        buff_.Append("[warn] : ", 9);
        break;
    case 3:
        buff_.Append("[error]: ", 9);
        break;
    default:
        buff_.Append("[info] : ", 9);
        break;
    }
}

void Log::Flush(){
    if (isAsync_){
        que_->flush();
    }
    fflush(fp_);
}

void Log::__AsyncWrite(){
    string str = "";
    while(que_->pop(str)){
        lock_guard<mutex>locker(mtx_);
        fputs(str.c_str(), fp_);
    }
}

Log* Log::Instance(){
    static Log inst;
    return &inst;
}

void Log::FlushLogThread(){
    Log::Instance()->__AsyncWrite();
}