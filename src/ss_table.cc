//---------------------------------------------------------------------------
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "tools/logger.h"
#include "tools/command_convert.h"
#include "command/set_command.h"
#include "command/rm_command.h"
#include "ss_table.h"
//---------------------------------------------------------------------------
namespace lsm
{

//---------------------------------------------------------------------------
SsTable::SsTable()
{
    fd_ = 0;
    offset_ = 0;
}
//---------------------------------------------------------------------------
SsTable::~SsTable()
{
    if(fd_)
    {
        ::close(fd_);
    }
}
//---------------------------------------------------------------------------
bool SsTable::Init(const std::string& path)
{
    Logger_debug("init ss-table from file:%s", path.c_str());

    path_ = path;
    fd_ = ::open(path.c_str(), O_RDONLY);
    if(fd_ < 0)
    {
        Logger_error("%s", tools::Logger::OsError(errno));
        return false;
    }

    // 1.读取元数据
    if(false == SsTableMetainfo::ReadFromFile(&metainfo_, fd_))
    {
        Logger_error("%s", tools::Logger::OsError(errno));
        return false;
    }

    // 2.读取稀疏索引
    if(false == ReadMetainfo())
    {
        Logger_error("read spare index failed!");
        return false;
    }

    return true;
}
//---------------------------------------------------------------------------
bool SsTable::Init(const std::string& path, size_t part_size, const IndexMap& index)
{
    Logger_debug("init ss-table from command:%s", path.c_str());

    fd_ = ::open(path.c_str(), O_RDWR|O_CREAT, 0660);
    if(fd_ < 0)
    {
        Logger_error("%s", tools::Logger::OsError(errno));
        return false;
    }
    path_ = path;
    metainfo_.set_part_size(part_size);

    // 持久化命令
    SaveCommand(index);

    // 更新元数据
    metainfo_.set_data_len(offset_);
    metainfo_.set_index_start(offset_);

    // 稀疏索引
    SaveSpareIndex();

    // 保存元数据
    metainfo_.WriteToFile(fd_);

    // 刷新到磁盘
    ::fdatasync(fd_);
    return true;
}
//---------------------------------------------------------------------------
bool SsTable::StartMerge(const std::string& path, size_t part_size)
{
    fd_ = ::open(path.c_str(), O_RDWR|O_CREAT, 0660);
    if(fd_ < 0)
    {
        Logger_error("%s", tools::Logger::OsError(errno));
        return false;
    }
    path_ = path;
    metainfo_.set_part_size(part_size);

    // 初始化分区
    part_data_ = Value(Value::Object);

    return true;
}
//---------------------------------------------------------------------------
bool SsTable::Merge(const std::shared_ptr<Command>& command)
{
    // 不判定是否重复（重复是上层逻辑处理的）

    // 插入
    part_data_[command->get_key()] = tools::CommandConvert::CommandToJson(command);

    // 到达分段大小,写入文件
    if(part_data_.Size() >= static_cast<size_t>(metainfo_.get_part_size()))
    {
        if(false == WriteDataPart())
        {
            return false;
        }
    }

    return true;
}
//---------------------------------------------------------------------------
bool SsTable::EndMerge()
{
    // 还有剩余数据，写入文件
    if(part_data_.Size() > 0)
    {
        if(false == WriteDataPart())
        {
            return false;
        }
    }

    // 更新元数据
    metainfo_.set_data_len(offset_);
    metainfo_.set_index_start(offset_);

    // 稀疏索引
    SaveSpareIndex();

    // 保存元数据
    metainfo_.WriteToFile(fd_);

    // 刷新到磁盘
    ::fdatasync(fd_);
    return true;
}
//---------------------------------------------------------------------------
std::shared_ptr<Command> SsTable::Query(const std::string& key) const
{
    // 1.查找第一个>key的索引
    auto iter = spare_index_.upper_bound(key);
    if(iter != spare_index_.end())
    {
        // 找到，所找的key可能在该索引的前面一个索引
        if(iter != spare_index_.begin())
        {
            iter--;
        }
    }
    else
    {
        // 没找到，查看最后一个索引有没有
        iter = --spare_index_.end();
    }

    // 2.读取分区数据
    auto& position = iter->second;
    std::string data_str;
    data_str.resize(position.get_length());
    if(::pread(fd_, const_cast<char*>(data_str.data()), position.get_length(), position.get_index()) < 0)
    {
        Logger_error("read data part filed:%s", tools::Logger::OsError(errno));
        return nullptr;
    }
    Value data = tools::CommandConvert::JsonStrToJson(data_str);
    if(data == Value::NullValue)
    {
        Logger_error("read sstable data failed!");
        return nullptr;
    }

    // 3.查询
    const Value& value = data.ObjectGet(key);
    if(Value::NullValue == value)
    {
        return nullptr;
    }

    return tools::CommandConvert::JsonToCommand(value);
}
//---------------------------------------------------------------------------
bool SsTable::ReadMetainfo()
{
    std::string spare_index_str;
    spare_index_str.resize(metainfo_.get_index_len());
    if(::pread(fd_, const_cast<char*>(spare_index_str.data()), spare_index_str.size(), metainfo_.get_index_start()) < 0)
    {
        Logger_error("read data part filed:%s", tools::Logger::OsError(errno));
        return false;
    }

    Value value;
    JsonReader reader;
    if(false == reader.Parse(spare_index_str, value))
    {
        return false;
    }
    for(auto iter=value.ObjectIterBegin(); iter!=value.ObjectIterEnd(); iter++)
    {
        const auto& second = iter->second;
        spare_index_.insert(std::make_pair(iter->first, Position(second["index"].AsUInt64(), second["len"].AsUInt64())));
    }

    return true;
}
//---------------------------------------------------------------------------
bool SsTable::SaveCommand(const IndexMap& index)
{
    // 初始化分区
    part_data_ = Value(Value::Object);

    for(const auto& item : index)
    {
        const auto& command = item.second;

        part_data_[command->get_key()] = tools::CommandConvert::CommandToJson(command);

        // 到达分段大小,写入文件
        if(part_data_.Size() >= static_cast<size_t>(metainfo_.get_part_size()))
        {
            if(false == WriteDataPart())
            {
                return false;
            }
        }
    }

    // 还有剩余数据，写入文件
    if(part_data_.Size() > 0)
    {
        if(false == WriteDataPart())
        {
            return false;
        }
    }

    return true;
}
//---------------------------------------------------------------------------
bool SsTable::SaveSpareIndex()
{
    Value spare_index(Value::Object);
    for(const auto& spare : spare_index_)
    {
        Value tmp(Value::Object);
        tmp["index"] = spare.second.get_index();
        tmp["len"] = spare.second.get_length();
        spare_index[spare.first] = tmp;
    }
    std::string str = JsonWriter(spare_index).ToString();
    if(::write(fd_, str.c_str(), str.length()) != static_cast<ssize_t>(str.length()))
    {
        return false;
    }
    metainfo_.set_index_len(str.length());

    return true;
}
//---------------------------------------------------------------------------
bool SsTable::WriteDataPart()
{
    // 写入文件
    std::string str = JsonWriter(part_data_).ToString();
    if(::write(fd_, str.c_str(), str.length()) != static_cast<ssize_t>(str.length()))
    {
        Logger_error("%s:%s", str.c_str(), tools::Logger::OsError(errno));
        return false;
    }

    // 记录数据段第一个key到稀疏索引中
    std::string key = part_data_.ObjectIterBegin()->first;
    spare_index_.insert(std::make_pair(key, Position(offset_, str.length())));
    offset_ += str.length();

    // 重置分区
    part_data_ = Value(Value::Object);
    return true;
}
//---------------------------------------------------------------------------

}//namespace lsm
//---------------------------------------------------------------------------
