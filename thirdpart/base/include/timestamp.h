//---------------------------------------------------------------------------
#ifndef BASE_TIMESTAMPK_H_
#define BASE_TIMESTAMPK_H_
//---------------------------------------------------------------------------
#include <string>
//---------------------------------------------------------------------------
/*
 * string time string format: YYYY-MM-DD HOUR:MINUTE:SECONDS
*/
//---------------------------------------------------------------------------
namespace base
{

class Timestamp
{
public:
    Timestamp()
    :   micro_seconds_(0)
    {
    }
    explicit Timestamp(uint64_t micro_seconds)
    :   micro_seconds_(micro_seconds)
    {
    }
    explicit Timestamp(const std::string& datetime);

    std::string Date();
    std::string Time();
    std::string Datetime(bool decimal=false);

    uint64_t Secodes()      { return micro_seconds_ / kMicrosecondsPerSecond; }
    uint64_t Microseconds() { return micro_seconds_; }

    Timestamp& AddTime      (uint64_t seconds);
    Timestamp& ReduceTime   (uint64_t seconds);


    static Timestamp    Now();
    static Timestamp&   Invalid();

public:
    static const int kMicrosecondsPerSecond = 1000 * 1000;
    static const int kSecondsPerMinute      = 60;
    static const int kSecondsPerHour        = kSecondsPerMinute * 60;
    static const int kSecondsPerDay         = kSecondsPerHour * 24;

private:
    uint64_t micro_seconds_;

    static Timestamp kInvalid;
};
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
inline bool operator == (Timestamp left, Timestamp right)
{
    return left.Microseconds() == right.Microseconds();
}
//---------------------------------------------------------------------------
inline bool operator != (Timestamp left, Timestamp right)
{
    return left.Microseconds() != right.Microseconds();
}
//---------------------------------------------------------------------------
inline bool operator > (Timestamp left, Timestamp right)
{
    return left.Microseconds() > right.Microseconds();
}
//---------------------------------------------------------------------------
inline bool operator < (Timestamp left, Timestamp right)
{
    return left.Microseconds() < right.Microseconds();
}
//---------------------------------------------------------------------------
inline uint64_t operator + (Timestamp left, Timestamp right)
{
    return left.Microseconds() + right.Microseconds();
}
//---------------------------------------------------------------------------
inline uint64_t operator - (Timestamp left, Timestamp right)
{
    return left.Microseconds() - right.Microseconds();
}
//---------------------------------------------------------------------------
}//namespace base
//---------------------------------------------------------------------------
#endif// BASE_TIMESTAMPK_H_
