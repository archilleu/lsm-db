//---------------------------------------------------------------------------
#ifndef BASE_NONCOPYABLE_H_
#define BASE_NONCOPYABLE_H_
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
namespace base
{

class Noncopyable
{
protected:
    Noncopyable() {}
    ~Noncopyable() {}

private:
    Noncopyable(const Noncopyable&);
    const Noncopyable& operator=(const Noncopyable&);
};


}//namespace net
//---------------------------------------------------------------------------
#endif //NET_EVENT_LOOP_H_
