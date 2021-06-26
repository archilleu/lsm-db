//---------------------------------------------------------------------------
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "tools/logger.h"
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
bool SsTable::init(const std::string& path)
{
    path_ = path;
    fd_ = ::open(path.c_str(), O_RDONLY);
    if(!fd_)
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
    std::string spare_index_str;
    spare_index_str.resize(metainfo_.get_index_len());
    ::pread(fd_, const_cast<char*>(spare_index_str.data()), spare_index_str.size(), metainfo_.get_index_start());

    Value value;
    JsonReader reader;
    if(false == reader.Parse(spare_index_str, value))
    {
        Logger_error("read spare index failed!");
        return false;
    }
    for(auto iter=value.ObjectIterBegin(); iter!=value.ObjectIterEnd(); iter++)
    {
        const auto& second = iter->second;
        Position position(second["index"].AsUInt64(), second["len"].AsUInt64());
        spare_index_.insert(std::make_pair(iter->first, position));
    }

    return true;
}
//---------------------------------------------------------------------------
bool SsTable::init(const std::string& path, size_t part_size, std::map<std::string, std::shared_ptr<Command>> index)
{
    fd_ = ::open(path.c_str(), O_WRONLY|O_CREAT, 0660);
    if(!fd_)
    {
        Logger_error("%s", tools::Logger::OsError(errno));
        return false;
    }
    path_ = path;
    metainfo_.set_part_size(part_size);

    // 持久化命令
    Value part_data(Value::Object);
    for(const auto& item : index)
    {
        const auto& command = item.second;

        // set
        if(command->get_type() == CommandType::SET)
        {
            SetCommand* set_command = dynamic_cast<SetCommand*>(command.get());
        
            Value tmp(Value::Object);
            tmp["type"] = 0;
            tmp["key"] = set_command->get_key();
            tmp["value"] = set_command->get_value();
            part_data[set_command->get_key()] = tmp;
        }

        // rm
        if(command->get_type() == CommandType::RM)
        {
            RmCommand* rm_command = dynamic_cast<RmCommand*>(command.get());
        
            Value tmp(Value::Object);
            tmp["type"] = 1;
            tmp["key"] = rm_command->get_key();
            part_data[rm_command->get_key()] = tmp;
        }

        // 到达分段大小,写入文件
        if(part_data.Size() >= static_cast<size_t>(metainfo_.get_part_size()))
        {
            WriteDataPart(part_data);
            part_data = Value(Value::Object);
        }
    }

    // 还有剩余数据，写入文件
    if(part_data.Size() > 0)
    {
        WriteDataPart(part_data);
    }

    // 更新元数据
    metainfo_.set_data_len(offset_);
    metainfo_.set_index_start(offset_);

    // 稀疏索引
    Value spare_index(Value::Object);
    for(const auto& spare : spare_index_)
    {
        Value tmp(Value::Object);
        tmp["index"] = spare.second.get_index();
        tmp["len"] = spare.second.get_length();
        spare_index[spare.first] = tmp;
    }
    std::string str = JsonWriter(spare_index).ToString();
    ::write(fd_, str.c_str(), str.length());
    metainfo_.set_index_len(str.length());

    // 保存元数据
    metainfo_.WriteToFile(fd_);

    return true;
}
//---------------------------------------------------------------------------
std::shared_ptr<Command> SsTable::Query(const std::string& key)
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

    // 读取分区数据
    auto& position = iter->second;
    std::string data_str;
    data_str.resize(position.get_length());
    ::pread(fd_, const_cast<char*>(data_str.data()), position.get_length(), position.get_index());
    Value data;
    JsonReader reader;
    if(false == reader.Parse(data_str, data))
    {
        Logger_error("read sstable data failed!");
        return nullptr;
    }
    const Value& value = data.ObjectGet(key);
    if(Value::NullValue == value)
    {
        return nullptr;
    }
    if(value["type"].AsUInt() == 0)
    {
        std::shared_ptr<Command> command = std::make_shared<SetCommand>(value["key"].AsString(), value["value"].AsString());
        return command;
    }
    else
    {
        std::shared_ptr<Command> command = std::make_shared<RmCommand>(value["key"].AsString());
        return command;
    }

    return nullptr;
}
//---------------------------------------------------------------------------
bool SsTable::WriteDataPart(const Value& part_data)
{
    // 写入文件
    std::string str = JsonWriter(part_data).ToString();
    ::write(fd_, str.c_str(), str.length());

    // 记录数据段第一个key到稀疏索引中
    std::string key = part_data.ObjectIterBegin()->first;
    spare_index_.insert(std::make_pair(key, Position(offset_, str.length())));
    offset_ += str.length();

    return true;
}
//---------------------------------------------------------------------------

}//namespace lsm
//---------------------------------------------------------------------------
