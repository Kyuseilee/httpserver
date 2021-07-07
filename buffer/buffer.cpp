/*
 * @Author: rosonlee 
 * @Date: 2021-07-06 20:49:01 
 * @Last Modified by: rosonlee
 * @Last Modified time: 2021-07-06 22:30:26
 */

#include "buffer.h"

Buffer::Buffer(int initBuffsize) : buffer_(initBuffsize), read_pos_(0), write_pos_(0){}

size_t Buffer::ReadableBytes() const {
    return buffer_.size() - read_pos_;
}

size_t Buffer::WritableBytes() const {
    return buffer_.size() - write_pos_;
}

size_t Buffer::PrependableBytes() const {
    return read_pos_;
}

const char* Buffer::Peek() const {
    return __BeginPtr() + read_pos_;
}

void Buffer::EnsureWritable(size_t len){
    if (WritableBytes() < len){
        __MakeSpace(len);
    }
    assert(WritableBytes() >= len);
}

void Buffer::HasWritten(size_t len){
    write_pos_ += len;
}

void Buffer::Retrieve(size_t len){
    assert(len <= ReadableBytes());
    read_pos_ += len;
}

void Buffer::RetrieveUntil(const char* end){
    assert(Peek() <= end);
    Retrieve(end - Peek());
}

void Buffer::RetrieveAll(){
    bzero(&buffer_[0], buffer_.size());
    read_pos_ = 0;
    write_pos_ = 0;
}

std::string Buffer::RetrieveAllToStr(){
    std::string str(Peek(), ReadableBytes());
    RetrieveAll();
    return str;
}

const char* Buffer::BeginWriteConst() const{
    return __BeginPtr() + write_pos_;
}
char* Buffer::BeginWrite() {
    return __BeginPtr() + write_pos_;
}

void Buffer::Append(const char* str, size_t len){
    assert(str);
    EnsureWritable(len);
    std::copy(str, str + len, BeginWrite());
    HasWritten(len);
}

void Buffer::Append(const std::string& str){
    Append(str.data(), str.size());
}

void Buffer::Append(const void* data, size_t len){
    assert(data);
    Append(static_cast<const char*>(data), len);
}

void Buffer::Append(const Buffer& buff){
    Append(buff.Peek(), buff.ReadableBytes());
}

size_t Buffer::ReadFd(int fd, int* saveErrno){
    char buff[65535];
    struct iovec iov[2];
    const size_t writable = WritableBytes();

    iov[0].iov_base = __BeginPtr() + write_pos_;
    iov[0].iov_len = writable;
    iov[1].iov_base = buff;
    iov[1].iov_len = sizeof(buff);
    
    const size_t len = readv(fd, iov, 2);
    if (len < 0){
        *saveErrno = errno;
    }
    else if (static_cast<size_t>(len) <= writable){
        write_pos_ += len;
    }
    else{
        write_pos_ = buffer_.size();
        Append(buff, len - writable);
    }
    return len;
}


size_t Buffer::WriteFd(int fd, int* saveErrno){
    size_t readSize = ReadableBytes();
    ssize_t len = write(fd, Peek(), readSize);
    if (len < 0){
        *saveErrno = errno;
        return len;
    }
    read_pos_ += len;
    return len;
}

char* Buffer::__BeginPtr(){
    return &*buffer_.begin();
}

const char* Buffer::__BeginPtr() const {
    return &*buffer_.begin();
}

void Buffer::__MakeSpace(size_t len){
    if (WritableBytes() + PrependableBytes() < len){
        buffer_.resize(write_pos_ + len + 1);
    }
    else{
        size_t readable = ReadableBytes();
        std::copy(__BeginPtr() + read_pos_, __BeginPtr() + write_pos_, __BeginPtr());
        read_pos_ = 0;
        write_pos_ = read_pos_ + readable;
        assert(readable == ReadableBytes());
    }
}

