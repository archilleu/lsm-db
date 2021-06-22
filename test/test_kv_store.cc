//---------------------------------------------------------------------------
#include <iostream>

#include "../src/kv_store.h"
//---------------------------------------------------------------------------
using namespace lsm;
//---------------------------------------------------------------------------
int main(int, char**)
{
    KvStore kv_store(".", 2, 1);

    kv_store.Set("key", "value");
    std::string key = kv_store.Get("key");
    std::cout << key << std::endl;

    kv_store.Rm("key");
    key = kv_store.Get("key");
    std::cout << key << std::endl;
}
