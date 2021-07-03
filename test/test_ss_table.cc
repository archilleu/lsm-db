//---------------------------------------------------------------------------
#include <iostream>
#include <list>

#include "../src/ss_table.h"
#include "../src/command/set_command.h"
#include "../src/command/rm_command.h"
#include "../src/tools/command_convert.h"

#include "../thirdpart/base/include/function.h"
//---------------------------------------------------------------------------
using namespace lsm;
//---------------------------------------------------------------------------
bool TestStore()
{
    // 构建命令
    int size = 10200;
    std::string key = "key";
    std::string value = "value";
    std::map<std::string, std::shared_ptr<Command>> index;
    for(int i=0; i<size; i++)
    {
        std::string key_tmp = base::CombineString("%s%d", key.c_str(), i);
        std::string rm_key_tmp = base::CombineString("rm_%s%d", key.c_str(), i);
        std::string value_tmp = base::CombineString("%s%d", value.c_str(), i);
        std::shared_ptr<SetCommand> set_command = std::make_shared<SetCommand>(key_tmp, value_tmp);
        std::shared_ptr<RmCommand> rm_command = std::make_shared<RmCommand>(rm_key_tmp);
        index.insert(std::make_pair(key_tmp, set_command));
        index.insert(std::make_pair(rm_key_tmp, rm_command));
    }

    // 持久化
    const char* path = "/tmp/test.txt";
    {
        SsTable ss_table;
        if(false == ss_table.Init(path, 13, index))
        {
            assert(0);
        }
    }

    // 查询ss-table
    {
        SsTable ss_table;
        if(false == ss_table.Init(path))
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
            std::string rm_key_tmp = base::CombineString("rm_%s%d", key.c_str(), i);
            std::string value_tmp = base::CombineString("%s%d", value.c_str(), i);

            std::shared_ptr<Command> command = ss_table.Query(key_tmp);
            std::shared_ptr<Command> rm_command = ss_table.Query(rm_key_tmp);
            assert(command);

            if(command->get_type() == CommandType::SET)
            {
                SetCommand* set_command = dynamic_cast<SetCommand*>(command.get());
                std::cout << tools::CommandConvert::CommandToJsonStr(command) << std::endl;
                if(key_tmp != set_command->get_key())
                {
                    assert(0);
                }
                if(value_tmp != set_command->get_value())
                {
                    assert(0);
                }
            }
            if(rm_command->get_type() == CommandType::RM)
            {
                RmCommand* rm_command1 = dynamic_cast<RmCommand*>(rm_command.get());
                std::cout << tools::CommandConvert::CommandToJsonStr(rm_command) << std::endl;
                if(rm_key_tmp != rm_command1->get_key())
                {
                    assert(0);
                }
            }
        }
    }

    return true;
}
//---------------------------------------------------------------------------
bool TestMerge()
{
    size_t size = 10240;
    size_t part_size = 130;
    const char* merge_path = "/tmp/merge.txt";
    {
        SsTable ss_table;
        if(false == ss_table.StartMerge(merge_path, part_size))
        {
            assert(0);
        }

        std::list<std::string> keys;
        for(size_t i=0; i<size; i++)
        {
            std::string key_tmp = base::CombineString("key%d", i);
            keys.push_back(key_tmp);

        }
        keys.sort();

        for(std::string key : keys)
        {
            std::shared_ptr<SetCommand> set_command = std::make_shared<SetCommand>(key, key);
            if(false == ss_table.Merge(set_command))
            {
                assert(0);
            }
        }

        if(false == ss_table.EndMerge())
        {
            assert(0);
        }
    }

    {
        SsTable ss_table;
        if(false == ss_table.Init(merge_path))
        {
            assert(0);
        }
        for(size_t i=0; i<size; i++)
        {
            std::string key_tmp = base::CombineString("key%d", i);
            auto command = ss_table.Query(key_tmp);
            std::cout << tools::CommandConvert::CommandToJsonStr(command) << std::endl;
            assert(command);
            assert(command->get_type() == CommandType::SET);
            SetCommand* set_command = dynamic_cast<SetCommand*>(command.get());
            if(key_tmp != set_command->get_key())
            {
                assert(0);
            }
        }
    }
    return true;
}
//---------------------------------------------------------------------------
int main(int, char**)
{
    TestStore();

    TestMerge();
    return 0;
}
