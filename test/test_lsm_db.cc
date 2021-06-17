//---------------------------------------------------------------------------
#include <iostream>

#include "../src/kv_store.h"
#include "../src/ss_table.h"
//---------------------------------------------------------------------------
using namespace lsm;
//---------------------------------------------------------------------------
int main(int, char**)
{
    KvStore kv_store = KvStore();
    const char* key = "abc";
    std::cout << kv_store.Get("key") << std::endl;
    std::cout << kv_store.Get(key) << std::endl;

    lsm::SsTable ss_table;
    std::cout << "hello lsm db" << std::endl;
}
