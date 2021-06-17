//---------------------------------------------------------------------------
#ifndef LSM_SET_COMMAND_H_
#define LSM_SET_COMMAND_H_
//---------------------------------------------------------------------------
#include "abstract_command.h"
//---------------------------------------------------------------------------
namespace lsm
{

class SetCommand : public AbstractCommand
{
public:
    SetCommand(const std::string& key, const std::string& value)
    :   AbstractCommand(CommandType::SET)
    {
        key_ = key;
        value_ = value;
    }

public:
    std::string get_key() { return key_; }
    std::string get_value() { return value_; }


private:
    std::string key_;
    std::string value_;
};


}//namespace lsm
//---------------------------------------------------------------------------
#endif //LSM_SET_COMMAND_H_
