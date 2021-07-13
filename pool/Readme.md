# ThreadPool

## 知识点：

1. *`explicit`* ：显示声明含有相似参数的函数的调用，即在输入为单个int的情况下，一定是调用该函数

2. `make_shared` 

   ```c++
   /*
   	1.提高性能，减少内存占用
   		对于shared_ptr:其要维护强引用和弱引用
   			强引用：用来记录当前有多少个存活的shared_ptrs正在持有该对象，当强引用为0时，将对象销毁/释放
   			弱引用：用来记录当前有多少个正在观察该对象的weak_ptrs， 最后一个弱引用离开后，共享的内部信息控制块被销毁和释放。
   		如果使用new 来进行shared_ptr的构造的话，控制块是单独的。
   		而如果使用make_shared，则其控制块会被统一放在一起。
   		
   		即：对于new shared_ptr，不仅要创建一个widget，还需要分配内存给控制块
   		    对于make_shared，只申请一块单独的内存来存放widget对象和控制块。
   		
   	2.异常安全
   		同样，对于两步分配内存，如果第一步正常，分配了空间而第二步失败，则构造失败，第一部分的空间却随之泄露。
   		
   	缺点：
   		1.构造函数时protected/private时，无法使用make_shared
   		2.weak_ptr也会保持控制块，导致只有所有的都将资源释放时，该部分内存才能被回收。
   */
   ```

   ![img](https://upload-images.jianshu.io/upload_images/14368201-179a5e7efec34eb2.png?imageMogr2/auto-orient/strip|imageView2/2/w/480/format/webp)

3. *`thread`* 库：

   ```c++
   #include<thread>
   
   初始化：
   //1:
    thread() noexcept;
   //2:
   template <class Fn, class... Args>
   explicit thread(Fn&& fn, Args&&... args);
   //3:
   thread(thread&& x) noexcept; //知识点： 右值引用
   
   /*！*/
   thread(const thread&) = delete;  //意味着拷贝构造函数被禁用，不允许拷贝创建线程
   ```

   在此处使用的是thread库来创建多线程，并使用detach分离

   > detach:  将当前线程对象所代表的执行实例与该线程对象分离，使得线程的执行可以单独进行。一旦线程执行完毕，它所分配的资源将会被释放。

4. 右值引用：

   左值和右值: 左值是能被赋予意义的，有明确地址的变量，可以出现在等号的左边或右边；右值是无地址的，中间变量，只能出现在等号右边。左值可以使用&进行地址访问。 

   在函数调用中，右值引用主要用来实现move语意，通过在参数中声明 `T&&` 对同名函数进行重载，使得编译器在编译阶段就可以知道对应的函数调用。

   * 右值引用可以使用const 连接常量： eg ：

     ```c++
     const int &ref = 10; //right
     //----------------------------------------------------------------------//
     void fnc(const int &x){
         //do nothing
     }
     
     int main(){
         fnc(10);//right
     }
     ```

5. 拷贝构造：

   1. 首先调用构造函数创建一个临时对象
   2. 销毁左侧原来的资源，并用临时对象的值代替它
   3. 销毁临时对象，释放资源

6. Move语意：

   1. 拷贝构造很麻烦，还会有额外的空间浪费，因此想使用类似swap的方法解决问题，交换二者的资源，并把交换后无用的对象销毁。
   2. std::move  将move()中的对象转变为右值引用，
   3. 移动构造，移动赋值，是有极高效率的。右值引用的价值也体现在这里。主题的思想是把把将亡值的资源接管过来。而不用自己去申请空空间。
   4. move调用告诉编译器：我们有一个左值，但是我们希望像处理一个右值一样去处理他，调用move就意味着承诺：除了对rr1赋值或者销毁它之外，我们将不能再使用它。在调用move之后，我们不能对移动后的源对象值做任何的假设。

7. s

8. `lambda`表达式

   ```c++
   /* 形式
   [ capture ] ( params ) opt -> ret { body; };
   	[] 不捕获任何变量。
   	[&] 捕获外部作用域中所有变量，并作为引用在函数体中使用（按引用捕获）。
   	[=] 捕获外部作用域中所有变量，并作为副本在函数体中使用（按值捕获）。
   	[=，&foo] 按值捕获外部作用域中所有变量，并按引用捕获 foo 变量。
   	[bar] 按值捕获 bar 变量，同时不捕获其他变量。
   	[this] 捕获当前类中的 this 指针，让 lambda 表达式拥有和当前类成员函数同样的访问权限。如果已经使用了 & 或者 =，就默认添加此选项。捕获 this 的目的是可以在 lamda 中使用当前类的成员函数和成员变量。
   */
   auto f = [](int a) -> int { return a + 1; };//example
   ```

9. 匿名函数体

   主要做的任务就是使用thread创建线程，不用匿名函数的话需要把函数体声明为static型（我直接写的时候报了一串很长非static的错）所以还是按照这个来做了。

10. `mutex, condition_variable`:

    ```c++
    mutex mtx_;
    lock_guard<mutex>locker(mtx_); //lock_guard和unique_lock都是智能锁，在调用结束后自动释放，但unique_lock的特点是可以在操作过程中手动加解锁。
    	//A lock_guard always holds a lock from its construction to its destruction. A unique_lock can be created without immediately locking, can unlock at any point in its existence, and can transfer ownership of the lock from one instance to another.
    	//https://stackoverflow.com/questions/20516773/stdunique-lockstdmutex-or-stdlock-guardstdmutex
    condition_variable cond_;
    cond_.notify_one();
    cond_.notify_all();
    cond_.wait();
    //基本条件变量用法，阻塞调用线程直到被通知恢复。
    
    //使用mtx_和cond_实现sem_；
    #include <thread>
    #include <condition_variable>
    using namespace std;
    class semaphore{
    public:
        void realease(){
            lock_guard<mutex>locker(mtx_);
            ++count_;
            cond_.notify_one();
        }
        void acquire(){
            lock_guard<mutex>locker(mtx_);
            while(!count_){
                cond_.wait(locker);
            }
            --count_;
        }
        bool try_acquire(){
            lock_guard<mutex>locker(mtx_);
            if(count_){
                --count;
                return true;
            }
            return false;
        }
        
    private:
        unsigned long count_;
        mutex mtx_;
        condition_variable cond_;
        
    };
    ```

11. `std::function<return type(paramas)>` ：c++中的函数指针 , 最大的用处是函数回调

    1. https://zhuanlan.zhihu.com/p/161356621
    2. https://en.cppreference.com/w/cpp/utility/functional/function

    ```c++
    //一个函数对象的容器的类型，用以存储任何可以以fun()的形式调用且返回类型为void的对象（函数，lambda函数，由std::bind产生的对象，以及任何其他重载了void operator()()的类），而不用关心这个对象实际是什么……
    ```

    

