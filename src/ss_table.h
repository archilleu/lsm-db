//---------------------------------------------------------------------------
#ifndef LSM_SS_TABLE_H_
#define LSM_SS_TABLE_H_
//---------------------------------------------------------------------------
#include <string>
#include <map>
#include <memory>

#include "../thirdpart/base/include/json/json.h"

#include "./command/command.h"
#include "ss_table_metainfo.h"
#include "position.h"
//---------------------------------------------------------------------------
using namespace base::json;
//---------------------------------------------------------------------------
namespace lsm
{

class SsTable
{
public:
    SsTable();
    ~SsTable();

public:
    bool init(const std::string& path);
    bool init(const std::string& path, size_t part_size, std::map<std::string, std::shared_ptr<Command>> index);

public:
    std::shared_ptr<Command> Query(const std::string& key);

private:
    bool WriteDataPart(const Value& part_data);

private:
    // 表元数据
    SsTableMetainfo metainfo_;

    //字段稀疏索引
    std::map<std::string, Position> spare_index_;

    // 文件句柄
    int fd_;

    // 文件路径
    std::string path_;

    // 写文件偏移
    size_t offset_;
};


}//namespace lsm
//---------------------------------------------------------------------------
#endif //LSM_SS_TABLE_H_
