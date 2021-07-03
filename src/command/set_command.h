//---------------------------------------------------------------------------
#ifndef LSM_SET_COMMAND_H_
#define LSM_SET_COMMAND_H_
//---------------------------------------------------------------------------
#include "../../thirdpart/base/include/json/json.h"

#include "command.h"
//---------------------------------------------------------------------------
namespace lsm
{

//---------------------------------------------------------------------------
using namespace base::json;
//---------------------------------------------------------------------------
class SetCommand : public Command
{
public:
    SetCommand(const std::string& key, const std::string& value)
    :   Command(CommandType::SET, key)
    {
        value_ = value;
    }

public:
    std::string get_value() { return value_; }

private:
    std::string value_;
};


}//namespace lsm
//---------------------------------------------------------------------------
#endif //LSM_SET_COMMAND_H_
