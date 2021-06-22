//---------------------------------------------------------------------------
#ifndef LSM_COMMAND_H_
#define LSM_COMMAND_H_
//---------------------------------------------------------------------------
#include <string>

#include "command_type.h"
//---------------------------------------------------------------------------
namespace lsm
{

class Command
{
public:
    Command()
    {}

    Command(CommandType type)
    {
        type_ = type;
    }
    virtual ~Command(){}

public:
    CommandType get_type() { return type_; }

    virtual std::string ToString() const { return ""; }

private:
    CommandType type_;
};


}//namespace lsm
//---------------------------------------------------------------------------
#endif //LSM_COMMAND_H_
