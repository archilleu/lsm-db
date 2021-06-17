//---------------------------------------------------------------------------
#ifndef LSM_KV_STORE_H_
#define LSM_KV_STORE_H_
//---------------------------------------------------------------------------
#include <string>
//---------------------------------------------------------------------------
namespace lsm
{

class KvStore
{
public:
    // 保存数据
    void Set(const std::string& key, const std::string& value);

    // 获取数据
    std::string Get(const std::string& key) const;

    // 删除数据
    void Rm(const std::string& key);
};


}//namespace lsm
//---------------------------------------------------------------------------
#endif //LSM_KV_STORE_H_
