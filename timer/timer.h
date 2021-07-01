/*
 * @Author: rosonlee 
 * @Date: 2021-07-01 20:45:46 
 * @Last Modified by: rosonlee
 * @Last Modified time: 2021-07-01 22:57:27
 */

/*
    ! 时间堆实现方式，使用小根堆实现定时器
    ! 在里面做了一步int—>size_t的转换，有必要也没有必要，主要是这样保证size_t传过去后不用每个函数都assert，该类型保证能容纳实现所建立的最大对象的字节大小
*/


#ifndef TIMER_H
#define TIMER_H

#include <queue>
#include <vector>
#include <unordered_map>
#include <time.h>
#include <algorithm>
#include <arpa/inet.h>
#include <functional>
#include <assert.h>
#include <chrono>

//#include "" //TODO LOG

typedef std::function<void()> TimeoutCallBack;
typedef std::chrono::high_resolution_clock Clock;
typedef std::chrono::milliseconds MS;
typedef Clock::time_point TimeStamp;

struct TimerNode{
    int id;
    TimeStamp expires;
    TimeoutCallBack cb;
    bool operator<(const TimerNode& t){
        return expires < t.expires;
    }
};

class HeapTimer
{
public:
    HeapTimer(){ heap_.reserve(64); };
    ~HeapTimer() { Clear(); };

    void Adjust(int id, int newExpires);

    void Add(int id, int timeOut, const TimeoutCallBack& cb);

    void DoWork(int id);

    void Clear();

    void Tick();

    void Pop();

    int GetNextTick();

private:
    void __ShiftUp(size_t i);
    bool __ShiftDown(size_t index, size_t n);
    void __Swap(size_t i, size_t j);

    void __Del(size_t index);

private:
    std::vector<TimerNode>heap_;
    std::unordered_map<int, size_t>ref_;
};



#endif // TIMER_H