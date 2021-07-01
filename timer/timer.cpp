/*
 * @Author: rosonlee 
 * @Date: 2021-07-01 20:45:39 
 * @Last Modified by: rosonlee
 * @Last Modified time: 2021-07-01 22:56:56
 */

#include "timer.h"

void HeapTimer::Adjust(int id, int newExpires){
    assert(!heap_.empty() && ref_.count(id) > 0);
    heap_[ref_[id]].expires = Clock::now() + MS(newExpires);
    __ShiftDown(ref_[id]);
}

void HeapTimer::Add(int id, int timeOut, const TimeoutCallBack& cb){
    assert(id >= 0);
    size_t i;
    if (!ref_.count(id)){
        i = heap_.size();
        ref_[id] = i;
        heap_.push_back({id, Clock::now() + MS(timeOut), cb});
        __ShiftUp(i);
    }
    else{
        i = ref_[id];
        heap_[i].expires = Clock::now() + MS(timeOut);
        heap_[i].cb = cb;
        if (!__ShiftDown(i, heap_.size())){
            __ShiftUp(i);
        }
    }
}

void HeapTimer::DoWork(int id){
    if (heap_.empty() || ref_.count(id) == 0)
        return;
    size_t i = ref_[id];
    TimerNode node = heap_[i];
    node.cb();
    __Del(i);
}

void HeapTimer::Clear(){
    ref_.clear();
    heap_.clear();
}

void HeapTimer::Tick(){
    if (heap_.empty()){
        return;
    }

    while (!heap_.empty()){
        TimerNode node = heap_.front();
        if (std::chrono::duration_cast<MS>(node.expires - Clock::now()).count() > 0) 
            break;
        node.cb();
        Pop();
    }
}

void HeapTimer::Pop(){
    assert(!heap_.empty());
    __Del(0);
}

int HeapTimer::GetNextTick(){
    Tick();
    size_t res = -1;
    if (!heap_.empty()){
        res = std::chrono::duration_cast<MS>(heap_.front().expires - Clock::now()).count();
        if (res < 0) {res = 0;};
    }
    return res;
}




void HeapTimer::__Del(size_t index){
    assert(!heap_.empty() && index >= 0 && index < heap_.size());
    size_t i = index;
    size_t n = heap_.size()-1;
    assert(i <= n);
    if (i < n){
        __Swap(i, n);
        if (!__ShiftDown(i, n)){
            __ShiftUp(i);
        }
    }
    ref_.erase(heap_.back().id);
    heap_.pop_back();
}

void HeapTimer::__ShiftUp(size_t i){
    assert(i >= 0 && i < heap_.size());
    size_t j = (i-1) / 2;
    while (j >= 0){
        if (heap_[j] < heap_[i]){
            break;
        }
        __Swap(i, j);
        i = j;
        j = (i-1) / 2;
    }
}

bool HeapTimer::__ShiftDown(size_t index, size_t n){
    assert(index >= 0 && index < heap_.size());
    assert(n >= 0 && n <= heap_.size());
    size_t i = index;
    size_t j = i*2 + 1;
    while (j < n){
        if (j+1 < n && heap_[j+1] < heap_[j]){
            j += 1;
        }
        if (heap_[i] < heap_[j]) break;
        __Swap(i, j);
        i = j;
        j = i*2 + 1;
    }
    return i > index;
}

void HeapTimer::__Swap(size_t i, size_t j){
    assert(i >= 0 && i < heap_.size());
    assert(j >= 0 && j < heap_.size());
    std::swap(heap_[i], heap_[j]);
    ref_[heap_[i].id] = i;
    ref_[heap_[j].id] = j;
}