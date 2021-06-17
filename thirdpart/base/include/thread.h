//---------------------------------------------------------------------------
#ifndef BASE_THREAD_H_
#define BASE_THREAD_H_
//---------------------------------------------------------------------------
#include <string>
#include <memory>
#include <thread>
#include <atomic>
#include <functional>
//---------------------------------------------------------------------------
namespace base
{

//---------------------------------------------------------------------------
//每个线程都需要自己的副本
namespace CurrentThread
{
    extern __thread int         t_cache_tid;
    extern __thread char        t_cache_tid_str[32];
    extern __thread const char* t_thread_name;

    void CacheTid();

    inline int tid()
    {
        if(__builtin_expect(0==t_cache_tid, 0))
            CacheTid();

        return t_cache_tid;
    }

    inline const char* tid_str()
    {
        return t_cache_tid_str;
    }

    inline const char* tname()
    {
        return t_thread_name;
    }

    bool IsMainThread();

}//namespace CurrentThread
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
/*
 * 如果不join直接退出，会有以下风险:
 * 当Thread是一个很快消失的对象{Threadt(fun);}类似如此作用域的类,
 * 类析构的时候会detach线程,thread内部变量tname_内存可能会被下一个thread重用，
 * CurrentThread::tname线程局部变量指向上一个地址，导致可能的错误，
 * 所以最好用join保证或者适当延长作用域
*/
class Thread
{
public:
    //https://en.cppreference.com/w/cpp/utility/functional/function
    //store the result of a call to std::bind(the function signature can be
    //void(arg1, arg2...)
    using ThreadFunc = std::function<void (void)>;

    Thread(ThreadFunc&& thread_func, const std::string& thread_name=std::string());
    Thread(Thread&& other);
    Thread(const Thread&) =delete;
    Thread& operator=(const Thread&) =delete;
    ~Thread();

    bool Start();

    //警告:Join不能再本线程调用，线程会异常退出
    void Join();

    int tid() const { return tid_; }
    const std::string&  name() const { return name_; }

private:
    void OnThreadFunc();

    void SetThreadName();

private:
    int tid_;
    std::string name_;

    bool joined_;
    bool started_;

    std::thread thread_;
    ThreadFunc thread_func_;

private:
    static std::atomic<int> thread_num_;
};

}//namespace base
//---------------------------------------------------------------------------
#endif //BASE_THREAD_H_
