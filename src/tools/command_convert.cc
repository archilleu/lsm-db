//---------------------------------------------------------------------------
#include "command_convert.h"

#include "../command/command.h"
#include "../command/set_command.h"
#include "../command/rm_command.h"
//---------------------------------------------------------------------------
namespace lsm
{

namespace tools
{
using namespace base::json;
//---------------------------------------------------------------------------
std::string CommandConvert::CommandToJsonStr(const std::shared_ptr<Command>& command)
{
    Value value = CommandToJson(command);
    return JsonWriter(value).ToString();
}
//---------------------------------------------------------------------------
base::json::Value CommandConvert::CommandToJson(const std::shared_ptr<Command>& command)
{
    Value value(Value::Object);
    value["type"] = static_cast<unsigned>(command->get_type());
    value["key"] = command->get_key();
    value["timestamp"] = command->get_timestamp().Microseconds();
    // set
    if(command->get_type() == CommandType::SET)
    {
        SetCommand* set_command = dynamic_cast<SetCommand*>(command.get());
        value["value"] = set_command->get_value();
    }
    // rm
    else
    {
    }

    return value;
}
//---------------------------------------------------------------------------
std::shared_ptr<Command> CommandConvert::JsonStrToCommand(const std::string& command_str)
{
    Value value;
    JsonReader reader;
    if(false == reader.Parse(command_str, value))
    {
        return nullptr;
    }

    return JsonToCommand(value);
}
//---------------------------------------------------------------------------
base::json::Value CommandConvert::JsonStrToJson(const std::string& command_str)
{
    Value value;
    JsonReader reader;
    reader.Parse(command_str, value);
    return value;
}
//---------------------------------------------------------------------------
std::shared_ptr<Command> CommandConvert::JsonToCommand(const base::json::Value& value)
{
    base::Timestamp timestamp(value["timestamp"].AsUInt64());
    if(value["type"].AsUInt() == static_cast<unsigned>(CommandType::SET))
    {
        return std::make_shared<SetCommand>(value["key"].AsString(), value["value"].AsString(), timestamp);
    }
    else
    {
        return std::make_shared<RmCommand>(value["key"].AsString(), timestamp);
    }
}
//---------------------------------------------------------------------------

}//namespace tools

}//namespace lsm
//---------------------------------------------------------------------------
