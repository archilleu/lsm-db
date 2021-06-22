//---------------------------------------------------------------------------

#include "set_command.h"
//---------------------------------------------------------------------------
namespace lsm
{

//---------------------------------------------------------------------------
using namespace base::json;
//---------------------------------------------------------------------------
SetCommand::SetCommand(const std::string& key, const std::string& value)
:   Command(CommandType::SET)
{
    key_ = key;
    value_ = value;
}
//---------------------------------------------------------------------------
std::string SetCommand::ToString() const
{
    Value value(Value::Object);
    value["key"] = key_;
    value["value"] = value_;
    value["type"] = 0;
    return JsonWriter(value).ToString();
}
//---------------------------------------------------------------------------

}//namespace lsm
//---------------------------------------------------------------------------
