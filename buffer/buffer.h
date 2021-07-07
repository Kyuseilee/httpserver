/*
 * @Author: rosonlee 
 * @Date: 2021-07-06 18:19:57 
 * @Last Modified by: rosonlee
 * @Last Modified time: 2021-07-06 21:09:11
 */

#ifndef BUFFER_H
#define BUFFER_H

#include <cstring>
#include <iostream>
#include <unistd.h>
#include <sys/uio.h>
#include <vector>
#include <atomic>
#include <assert.h>

class Buffer{
public:
    Buffer(int initBuffSize = 1024);
    ~Buffer() = default;

    size_t WritableBytes() const;
    size_t ReadableBytes() const;
    size_t PrependableBytes() const;

    const char* Peek() const;
    void EnsureWritable(size_t len);
    void HasWritten(size_t len);

    void Retrieve(size_t len);
    void RetrieveUntil(const char* end);
    void RetrieveAll();
    std::string RetrieveAllToStr();

    const char* BeginWriteConst() const;
    char* BeginWrite();

    void Append(const char* str, size_t len);
    void Append(const std::string& str);
    void Append(const void* data, size_t len);
    void Append(const Buffer& buff);

    size_t ReadFd(int fd, int* Errno);
    size_t WriteFd(int fd, int* Errno);

private:
    char* __BeginPtr();
    const char* __BeginPtr() const ;
    void __MakeSpace(size_t len);

private:
    std::vector<char>buffer_;
    std::atomic<std::size_t>read_pos_;
    std::atomic<std::size_t>write_pos_;
};

#endif // BUFFER_H