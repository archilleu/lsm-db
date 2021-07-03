//---------------------------------------------------------------------------
#include <iostream>

#include "../thirdpart/base/include/json/json.h"

#include "../src/tools/command_convert.h"
#include "../src/command/command.h"
#include "../src/command/set_command.h"
#include "../src/command/rm_command.h"
//---------------------------------------------------------------------------
using namespace lsm;
using namespace lsm::tools;
//---------------------------------------------------------------------------
int main(int, char**)
{
    std::string key = "key";
    std::string value = "value";
    std::string rm_key = "rm_key";
    std::shared_ptr<Command> set_command = std::make_shared<SetCommand>(key, value);
    std::shared_ptr<Command> rm_command = std::make_shared<RmCommand>(rm_key);

    std::string set_str = CommandConvert::CommandToJsonStr(set_command);
    std::string rm_str = CommandConvert::CommandToJsonStr(rm_command);
    std::cout << "set:" << set_str << std::endl;
    std::cout << "rm:" << rm_str << std::endl;

    set_command = CommandConvert::JsonStrToCommand(set_str);
    if(set_command->get_type() != CommandType::SET)
    {
        assert(0);
    }
    else
    {
        auto command =  dynamic_cast<SetCommand*>(set_command.get());
        if(command->get_key() != key)
        {
            assert(0);
        }
        if(command->get_value() != value)
        {
            assert(0);
        }
    }

    rm_command = CommandConvert::JsonStrToCommand(rm_str);
    if(rm_command->get_type() != CommandType::RM)
    {
        assert(0);
    }
    else
    {
        auto command =  dynamic_cast<RmCommand*>(rm_command.get());
        if(command->get_key() != rm_key)
        {
            assert(0);
        }
    }

    return 0;
}
