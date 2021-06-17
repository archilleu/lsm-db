//---------------------------------------------------------------------------
#ifndef BASE_THREAD_LOCAL_H_
#define BASE_THREAD_LOCAL_H_
//---------------------------------------------------------------------------
#include <assert.h>
#include <pthread.h>
//---------------------------------------------------------------------------
namespace base
{

template<typename T>
class ThreadLocal
{
public:
    ThreadLocal()
    {
        //除非同名的key，否者一般不会失败（同名key在debug阶段就可以发现）
        if(0 != pthread_key_create(&key_, &ThreadLocal::Destructor))
            assert(0);

        return;
    }
    ~ThreadLocal()
    {
        if(0 != pthread_key_delete(key_))
            assert(0);

        return;
    }

    T& value()
    {
        T* val = static_cast<T*>(pthread_getspecific(key_));
        if(0 == val)
        {
            val = new T();
            if(0 != pthread_setspecific(key_, val))
            {
                assert(0);
                delete val;
                val = 0;
            }
        }

        return *val;
    }

private:
    static void Destructor(void* val)
    {
        T* obj = static_cast<T*>(val);
        if(0 == obj)
        {
            assert(0);
            return;
        }

        delete obj;
    }

private:
    pthread_key_t key_;
};

}//namespace base
//---------------------------------------------------------------------------
#endif//BASE_THREAD_LOCAL_H_
