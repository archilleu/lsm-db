//---------------------------------------------------------------------------
#include <ctime>

#include "../thirdpart/base/include/function.h"

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

    // FIXME: 细化锁
    std::lock_guard<std::mutex> guard(mutex_);

    // 1.写到wal文件
    WriteToWalFile(command);

    // 2.写到index
    index_[key] = command;

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
        if(false  == SwitchIndex())
        {
            return;
        }


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
bool KvStore::SwitchIndex()
{
    // 1. 把数据转移到不可变内存表中
    assert(((void)"imutable_index not null", immutable_index_.empty()));
    index_.swap(immutable_index_);

    // 2. 切换wal文件
    ::fclose(wal_file_);
    std::string old_path = data_dir_ + "/" + WAL;
    std::string new_path = data_dir_ + "/" + WAL_TMP;
    if(-1 == ::rename(old_path.c_str(), new_path.c_str()))
    {
        //重命名失败
        assert(0);
        return false;
    }

    // 3. 重新生成wal文件
    wal_file_ = ::fopen(old_path.c_str(), RW_MODE);
    return true;
}
//---------------------------------------------------------------------------
bool KvStore::StoreToSsTable()
{
    // 1. 按照时间戳生成sstable
    std::time_t t = std::time(0);    
    std::string path = base::CombineString("%s/%ld%s%", data_dir_.c_str(), "/", t, EXT);
    SsTable ss;
    // 清理

    // 2. 添加到sstable列表头部
    ss_tables_.push_front(ss);
    (void)path;

    return true;
}
//---------------------------------------------------------------------------

}//namespace lsm
//---------------------------------------------------------------------------
