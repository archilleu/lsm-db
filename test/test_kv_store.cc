//---------------------------------------------------------------------------
#include <iostream>
#include "../thirdpart/base/include/function.h"
#include "../thirdpart/base/include/logger.h"

#include "../src/command/set_command.h"
#include "../src/command/rm_command.h"

#include "../src/kv_store.h"

//---------------------------------------------------------------------------
using namespace lsm;
//---------------------------------------------------------------------------
int size = 1000;
size_t store_threshold = 37;
size_t part_size = 7;
std::string key = "key";
std::string value = "value";
//---------------------------------------------------------------------------
void TestKvStore()
{
    KvStore kv_store("/tmp/data", store_threshold, part_size);
    base::Logger::stdout_logger_mt();
    auto logger = base::Logger::file_stdout_logger_mt("/tmp/lsm-log", true);
    kv_store.SetLogger(logger);
    if(false == kv_store.Init())
    {
        std::cout << "init failed" << std::endl;
        assert(0);
        return;
    }

    for(int i=0; i<size; i++)
    {
        std::string key_tmp = base::CombineString("%s%d", key.c_str(), i);
        std::string value_tmp = base::CombineString("%s%d", value.c_str(), i);
        if(false == kv_store.Set(key_tmp, value_tmp))
        {
            assert(0);
        }
        std::cout << "set" << key_tmp << " " << value_tmp << std::endl;
    }

    for(int i=0; i<size; i++)
    {
        std::string key_tmp = base::CombineString("%s%d", key.c_str(), i);
        std::string value_tmp = base::CombineString("%s%d", value.c_str(), i);

        std::string get_value = kv_store.Get(key_tmp);
        assert(value_tmp == get_value);
    }

    for(int i=0; i<size; i++)
    {
        std::string key_tmp = base::CombineString("%s%d", key.c_str(), i);
        if(false == kv_store.Rm(key_tmp))
        {
            assert(0);
        }
    }

    for(int i=0; i<size; i++)
    {
        std::string key_tmp = base::CombineString("%s%d", key.c_str(), i);

        std::string get_value = kv_store.Get(key_tmp);
        assert(get_value.empty());
    }

}
//---------------------------------------------------------------------------
int main(int, char**)
{
    TestKvStore();

    return 0;
}
//---------------------------------------------------------------------------