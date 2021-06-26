//---------------------------------------------------------------------------
#ifndef LSM_POSITION_H_
#define LSM_POSITION_H_
//---------------------------------------------------------------------------
#include <cstddef>
//---------------------------------------------------------------------------
namespace lsm
{

class Position
{
public:
    Position(size_t index, size_t length)
    {
        index_ = index;
        length_ = length;
    }

public:
    size_t get_index() const { return index_; }
    size_t get_length() const { return length_; }

private:
    size_t index_;
    size_t length_;
};


}//namespace lsm
//---------------------------------------------------------------------------
#endif //LSM_POSITION_H_
