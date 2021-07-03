//---------------------------------------------------------------------------
#ifndef LSM_TOOLS_COMMAND_CONVERT_H_
#define LSM_TOOLS_COMMAND_CONVERT_H_
//---------------------------------------------------------------------------
#include <memory>

#include "../../thirdpart/base/include/json/json.h"
//---------------------------------------------------------------------------
namespace lsm
{

class Command;

namespace tools
{

class CommandConvert
{
public:
    static std::string CommandToJsonStr(const std::shared_ptr<Command>& command);

    static base::json::Value CommandToJson(const std::shared_ptr<Command>& command);

    static std::shared_ptr<Command> JsonStrToCommand(const std::string& command_str);

    static base::json::Value JsonStrToJson(const std::string& command_str);

    static std::shared_ptr<Command> JsonToCommand(const base::json::Value& value);
};

}//namespace tools

}//namespace lsm

//---------------------------------------------------------------------------
#endif //LSM_TOOLS_COMMAND_CONVERT_H_