/*
 * @Author: rosonlee 
 * @Date: 2021-06-28 19:23:07 
 * @Last Modified by: rosonlee
 * @Last Modified time: 2021-07-07 20:05:42
 */

#ifndef LOG_H
#define LOG_H

#include <mutex>
#include <condition_variable>
#include <thread>
#include <sys/time.h>
#include <string.h>
#include <stdarg.h>
#include <assert.h>
#include <sys/stat.h>

#include "block_queue.h"
#include "../buffer/buffer.h"

class Log{
public:
    void Init(int level, const char* path = "./log",
                const char* suffix = ".log", 
                int maxQueueCapacity = 1024);
    static Log* Instance();

    static void FlushLogThread();

    void Write(int level, const char* format, ...);
    void Flush();

    int GetLevel();
    void SetLevel(int level);
    bool IsOpen();

private:
    Log();
    void __AppendLogLevelTitle(int level);
    virtual ~Log();
    void __AsyncWrite();

private:
    static const int LOG_PATH_LEN = 256;
    static const int LOG_NAME_LEN = 256;
    static const int MAX_LINES = 50000;

    const char* path_;
    const char* suffix_;

private:
    int MAX_LINES_;

    int lineCount_;
    int toDay_;

    bool isOpen_;

    Buffer buff_;
    int level_;
    bool isAsync_;

    FILE* fp_;
    std::unique_ptr<BlockQueue<std::string>>que_;
    std::unique_ptr<std::thread>writeThread_;
    std::mutex mtx_;
};

#define LOG_BASE(level, format, ...) \
        do {\
            Log* log = Log::Instance();\
            if (log->IsOpen() && log->GetLevel() <= level){\
                log->Write(level, format, ##__VA__ARGS__);\
                log->Flush();\
            }\
        }while(0);

#define LOG_DEBUG(format, ...) do {LOG_BASE(0, format, ##__VA_ARGS__)} while(0);
#define LOG_INFO(format, ...) do {LOG_BASE(1, format, ##__VA_ARGS__)} while(0);
#define LOG_WARN(format, ...) do {LOG_BASE(2, format, ##__VA_ARGS__)} while(0);
#define LOG_ERROR(format, ...) do {LOG_BASE(3, format, ##__VA_ARGS__)} while(0);
#endif // LOG_H