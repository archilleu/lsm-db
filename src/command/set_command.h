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
    SetCommand(const std::string& key, const std::string& value);

    std::string ToString() const;

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
