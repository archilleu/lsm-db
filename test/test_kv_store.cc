//---------------------------------------------------------------------------
#include <iostream>
#include "../src/command/set_command.h"
#include "../src/command/rm_command.h"

#include "../src/kv_store.h"

#include "../thirdpart/base/include/function.h"
//---------------------------------------------------------------------------
using namespace lsm;
//---------------------------------------------------------------------------
int main(int, char**)
{
    KvStore kv_store("./data", 1, 1);

    int size = 10240;
    std::string key = "key";
    std::string value = "value";
    for(int i=0; i<size; i++)
    {
        std::string key_tmp = base::CombineString("%s%d", key.c_str(), i);
        std::string value_tmp = base::CombineString("%s%d", value.c_str(), i);
        kv_store.Set(key_tmp, value_tmp);
    }


    for(int i=0; i<size; i++)
    {
        std::string key_tmp = base::CombineString("%s%d", key.c_str(), i);
        std::string rm_key_tmp = base::CombineString("rm_%s%d", key.c_str(), i);
        std::string value_tmp = base::CombineString("%s%d", value.c_str(), i);

        std::string get_value = kv_store.Get(key_tmp);
        std::cout << get_value << std::endl;
        assert(value_tmp == get_value);
    }

    return 0;
}
