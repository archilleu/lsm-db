//---------------------------------------------------------------------------
#ifndef LSM_SS_TABLE_H_
#define LSM_SS_TABLE_H_
//---------------------------------------------------------------------------
#include <string>
#include <map>
#include <set>
#include <memory>

#include "../thirdpart/base/include/json/json.h"
#include "../thirdpart/base/include/noncopyable.h"

#include "./command/command.h"
#include "ss_table_metainfo.h"
#include "position.h"
//---------------------------------------------------------------------------
using namespace base::json;
//---------------------------------------------------------------------------
namespace lsm
{

class SsTable : public base::Noncopyable
{
public:
    SsTable();
    ~SsTable();

public:
    using IndexMap = std::map<std::string, std::shared_ptr<Command>>;

    // 初始化
    bool Init(const std::string& path);
    bool Init(const std::string& path, size_t part_size, const IndexMap& index);

    // 查询
    std::shared_ptr<Command> Query(const std::string& key) const;

    // 遍历全部数据
    bool Begin();
    bool HasNext();
    std::shared_ptr<Command> Next();

    // 合并
    bool StartMerge(const std::string& path, size_t part_size);
    bool Merge(const std::shared_ptr<Command>& command);
    bool EndMerge();

    std::string get_path() const { return path_; }

private:
    bool ReadMetainfo();

    bool SaveCommand(const IndexMap& index);

    bool SaveSpareIndex();

    bool WriteDataPart();

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

    // 保存的时候临时分区数据存储
    Value part_data_;

    // 遍历
    std::set<Position> positions_;
    std::set<Position>::const_iterator cur_position_;
    Value cur_part_data_;
    Value::ArrayValueIter cur_position_iter_;
};


}//namespace lsm
//---------------------------------------------------------------------------
#endif //LSM_SS_TABLE_H_
