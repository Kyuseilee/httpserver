# Timer
> 基于最小堆实现的定时器，内部结构是一个*struct*结构体，包括当前用户的id、回调函数，过期时间以及重载的`<`号。
> 
> `timer`的功能就是将新连接的用户添加到timer容器中，根据时间滴答来摘除掉过期的用户
## 知识点：

1. 堆排序
```c++ 
void adjustHeap(vector<int>&nums, int start, int end){
    int i = 2*start + 1;
    int tmp = nums[start];
    while (i <= end){
        if (i +1 < nums.size() && nums[i] < nums[i+1]){
            ++i;
        }
        if (nums[i] <= tmp){
            break;
        }
        nums[start] = nums[i];
        start = i;
        i = 2*start + 1;
    }
    nums[start] = tmp;
    return ;
}
void HeapSort(vector<int>&nums){
    int n = nums.size();
    for (int i = n/2-1; i >= 0; --i){
        adjustHeap(nums, i, n-1);
    }
    for (int i = n-1; i > 0; --i){
        swap(nums[0], nums[i]);
        adjustHeap(nums, 0, i-1);
    }
}

int main(){
    vector<int>nums{1, 4,2, 6, 9, 0, 21, 94, 45, 8};
    HeapSort(nums);
    for(int num : nums){
        cout << num << " ";
    }
    cout << endl;
    return 0;
}
```

2. 操作符重载：
如果要比较两个node属性，通过重载操作符来对其进行排序/选择操作。
3. `chrono`
```c++
#include <chrono>

//duration、time_point、clock


//1. std::chrono::duration 表示一段时间，
template<class Rep, class Period=ratio<1>>class duration;
/*
    其中
    Rep表示一种数值类型，用来表示Period的数量，比如int float double
    Period是ratio类型，用来表示【用秒表示的时间单位】比如second milisecond
    常用的duration<Rep,Period>已经定义好了，在std::chrono::duration下：
    ratio<3600, 1>              hours
    ratio<60, 1>                minutes
    ratio<1, 1>                 seconds
    ratio<1, 1000>              microseconds
    ratio<1, 1000000>           microseconds
    ratio<1, 1000000000>        nanosecons
*/
//std::duration_cast
std::duration_cast<type>(params);

//2.time points
/*
	std::chrono::time_point表示一个具体时间，
*/
std::time_point_cast<type>(params);

//3.clocks
/*
	std::chrono::system_clock 它表示当前的系统时钟，系统中运行的所有进程使用now()得到的时间是一致的。
    每一个clock类中都有确定的time_point, duration, Rep, Period类型。
    操作有：
    now() 当前时间time_point
    to_time_t() time_point转换成time_t秒
    from_time_t() 从time_t转换成time_point
    std::chrono::high_resolution_clock 顾名思义，这是系统可用的最高精度的时钟。实际上			  
    high_resolution_clock只不过是system_clock或者steady_clock的typedef。
*/
```