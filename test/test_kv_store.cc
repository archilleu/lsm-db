//---------------------------------------------------------------------------
#include <iostream>

#include "../src/kv_store.h"

#include "../thirdpart/base/include/function.h"
//---------------------------------------------------------------------------
using namespace lsm;
//---------------------------------------------------------------------------
int main(int, char**)
{
    KvStore kv_store(".", 2, 1);

    std::string key = "key";
    std::string value = "value";
    for(int i=0; i<5; i++)
    {
        kv_store.Set(base::CombineString("%s%d", key.c_str(), i),
                base::CombineString("%s%s", value.c_str(), i));
    }

    for(int i=0; i<5; i++)
    {
        std::string val = kv_store.Get(base::CombineString("%s%d", key.c_str(), i));
        std::cout << val << std::endl;
    }

}
