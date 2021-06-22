//---------------------------------------------------------------------------
#include "kv_store.h"
#include "command/set_command.h"
#include "command/rm_command.h"
#include "./tools/logger.h"
//---------------------------------------------------------------------------
namespace lsm
{

//---------------------------------------------------------------------------
const char* KvStore::EXT = ".table";
const char* KvStore::WAL = "wal";
const char* KvStore::RW_MODE = "wb";
const char* KvStore::WAL_TMP = "wal-tmp";
//---------------------------------------------------------------------------
KvStore::KvStore(std::string data_dir, size_t store_threshold, size_t part_size)
{
    data_dir_ = data_dir;
    store_threshold_ = store_threshold;
    part_size_ = part_size;

    // TODO: 加载ss table 目录
    //
    
    std::string path = data_dir + "/" + WAL;
    wal_file_ = ::fopen(path.c_str(), RW_MODE);
    return;
}
//---------------------------------------------------------------------------
KvStore::~KvStore()
{
    ::fclose(wal_file_);
}
//---------------------------------------------------------------------------
void KvStore::Set(const std::string& key, const std::string& value)
{
    std::shared_ptr<SetCommand> command = std::make_shared<SetCommand>(key, value);

    {
    std::lock_guard<std::mutex> guard(mutex_);

    // 1.写到wal文件
    WriteToWalFile(command);

    // 2.写到index
    index_[key] = command;
    }

    // 3.如果超过阈值进行持久化
    if(index_.size() > store_threshold_)
    {
        // TODO: 持久化
    }

    return;
}
//---------------------------------------------------------------------------
std::string KvStore::Get(const std::string& key) const
{
    // 1.先从索引中查找
    auto it = index_.find(key);

    // 2.可能在持久化过程中, 尝试从不可变索引中查找
    if(index_.end() == it && !immutable_index_.empty())
    {
        it = immutable_index_.find(key);

        // 3.还找不到，尝试从文件中查找
        if(immutable_index_.end() == it)
        {
            // TODO: 从ss table中查找
            
            return "";
        }
    }

    // 找到
    Command* command = it->second.get();
    if(command->get_type() == CommandType::SET)
    {
        SetCommand* set = static_cast<SetCommand*>(command);
        return set->get_value();
    }

    if(command->get_type() == CommandType::RM)
    {
        return "";
    }

    return "";
}
//---------------------------------------------------------------------------
void KvStore::Rm(const std::string& key)
{
    std::shared_ptr<RmCommand> command = std::make_shared<RmCommand>(key);

    {
    std::lock_guard<std::mutex> guard(mutex_);

    // 1.写到wal文件
    WriteToWalFile(command);

    // 2.写到index
    index_[key] = command;
    }

    // 3.如果超过阈值进行持久化
    if(index_.size() > store_threshold_)
    {
        // TODO: 持久化
    }

    return;
}
//---------------------------------------------------------------------------
bool KvStore::WriteToWalFile(const std::shared_ptr<Command>& command)
{
    std::string command_str = command->ToString();
    size_t size = command_str.size();
    ::fwrite(&size, sizeof(size), 1, wal_file_);
    ::fwrite(command_str.c_str(), size, 1, wal_file_);

    return true;
}
//---------------------------------------------------------------------------

}//namespace lsm
//---------------------------------------------------------------------------
