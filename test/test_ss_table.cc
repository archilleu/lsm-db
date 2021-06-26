//---------------------------------------------------------------------------
#include <iostream>

#include "../src/ss_table.h"
#include "../src/command/set_command.h"

#include "../thirdpart/base/include/function.h"
//---------------------------------------------------------------------------
using namespace lsm;
//---------------------------------------------------------------------------
int main(int, char**)
{
    int size = 1024;
    std::string key = "key";
    std::string value = "value";
    std::map<std::string, std::shared_ptr<Command>> index;
    for(int i=0; i<size; i++)
    {
        std::string key_tmp = base::CombineString("%s%d", key.c_str(), i);
        std::string value_tmp = base::CombineString("%s%d", value.c_str(), i);
        std::shared_ptr<SetCommand> set_command = std::make_shared<SetCommand>(key_tmp, value_tmp);
        index.insert(std::make_pair(key_tmp, set_command));
    }

    const char* path = "./test.txt";
    {
        SsTable ss_table;
        if(false == ss_table.init(path, 5, index))
        {
            assert(0);
        }
    }

    {
        SsTable ss_table;
        if(false == ss_table.init(path))
        {
        }
        // 没找到

        if(ss_table.Query("not find"))
        {
            assert(0);
        }

        for(int i=0; i<size; i++)
        {
            std::string key_tmp = base::CombineString("%s%d", key.c_str(), i);
            std::string value_tmp = base::CombineString("%s%d", value.c_str(), i);

            std::shared_ptr<Command> command = ss_table.Query(key_tmp);
            assert(command);

            if(command->get_type() == CommandType::SET)
            {
                SetCommand* set_command = dynamic_cast<SetCommand*>(command.get());
                std::cout << set_command->ToString() << std::endl;
                assert(key_tmp == set_command->get_key());
                assert(value_tmp == set_command->get_value());
            }
        }
    }


    return 0;
}
