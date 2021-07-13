# BlockQueue & Log

## *BlockQueue*

> 生产者消费者模式的阻塞队列，生产者使用其中的*push*，消费者使用其中的*pop*，数据结构用队列实现，一段进，一端出，提供几个基于*queue*本身功能的结构访问接口。

`std::cv_status`

​	带作用域枚举 `std::cv_status` 描述定时等待是否因时限返回,为 [std::condition_variable](https://www.apiref.com/cpp-zh/cpp/thread/condition_variable.html) 和 [std::condition_variable_any](https://www.apiref.com/cpp-zh/cpp/thread/condition_variable_any.html) 的 `wait_for` 和 `wait_until` 方法所用。

```c++
no_timeout	条件变量因 notify_all 、 notify_one 或虚假地被唤醒
timeout	条件变量因时限耗尽被唤醒
```

## Log

> 单例模式(饿汉)实现的（异步）线程安全Log，提供不同等级的日志写入，支持flush刷新缓冲区，

1. `snprintf()`

   ```c++
   int snprintf(char *str, int n, char * format [, argument, ...]);
   //str为要写入的字符串；n为要写入的字符的最大数目，超过n会被截断；format为格式化字符串，与printf()函数相同；argument为变量。
   ```

2. 无前缀的大括号体：

   表示生存周期，括号里的变量生命周期只在括号内，相当于划分出的需要紧密结合的一部分事务。

3. 单例模式：

```c++
//懒汉式，使用才创建，线程不安全
class singleton{
private:
    staic singleton* local_sigleton;
    singleton();
    ~singleton();
public:
    static singleton* GetInstance(){
        if(local_singleton == nullptr){
            local_singleton = new singleton();
        }
        return local_singleton;
    }
};
//改进懒汉式 
//https://blog.csdn.net/u012940886/article/details/80386669
 class singleton   //实现单例模式的类  
  {  
  private:  
      singleton(){}  //私有的构造函数  
      static singleton* Instance;  
        
  public:  
      static singleton* GetInstance()  
      {  
          if (Instance == NULL) //判断是否第一调用  
          {   
              Lock(); //表示上锁的函数  
              if (Instance == NULL)  
              {  
                  Instance = new singleton();  
              }  
              UnLock() //解锁函数  
          }             
          return Instance;  
      }  
  };  

//饿汉式，类内即创建，调用直接返回
class singleton{
private:
    singleton(){};
public:
    static singleton* GetInstance(){
        static singleton Instance;
        return &Instance;
    }
}
```

4. `fflush、fputs`

   ```c++
   //fflush()用于清空文件缓冲区，如果文件是以写的方式打开 的，则把缓冲区内容写入文件。其原型为：
   int fflush(FILE* stream);
   
   //s 代表要输出的字符串的首地址，可以是字符数组名或字符指针变量名,stream 表示向何种流中输出，可以是标准输出流 stdout，也可以是文件流
   int fputs(const char *s, FILE *stream);
   
   //http://c.biancheng.net/view/238.html
   //fputs() 和 puts() 有两个小区别：
   //puts() 只能向标准输出流输出，而 fputs() 可以向任何流输出。
   //使用 puts() 时，系统会在自动在其后添加换行符；而使用 fputs() 时，系统不会自动添加换行符。
   ```

5. `va_list`

   ```c++
   //可变长字符串，用于向字符串中打印数据、数据格式用户自定义
   #include <stdarg.h>
   
   va_list vaList;
   
   va_start(vaList, format);
   int _vsnprintf(char* str, size_t size, const char* format, va_list ap);
   va_end(vaList)
   ```

6. `##__VA_ARGS__`

   在宏中使用的可变长参数，允许可以定义可变参数宏(variadic macros)，使用保留名 __VA_ARGS__ 把参数传递给宏。当宏的调用展开时，实际的参数就传递给 printf()了。