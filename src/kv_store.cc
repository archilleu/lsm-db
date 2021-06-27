//---------------------------------------------------------------------------
#include <ctime>
#include <cstring>
#include <cstdlib>

#include <sys/types.h>
#include <dirent.h>

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
const char* KvStore::RW_MODE = "a+";
const char* KvStore::WAL_TMP = "wal_tmp";
//---------------------------------------------------------------------------
KvStore::KvStore(const std::string& data_dir, size_t store_threshold, size_t part_size)
{
    data_dir_ = data_dir;
    store_threshold_ = store_threshold;
    part_size_ = part_size;

    // 生成ss table目录
    if(!base::FolderExist(data_dir_))
    {
        if(false == base::FolderCreate(data_dir, true))
        {
            Logger_error("create data folder failed!");
            abort();
        }
    }

    // 加载ss table
    std::list<std::string> files = GetSsTableFileList();
    for(std::string file : files)
    {
        // TODO: wap tmp异常文件处理
        if(file == WAL_TMP)
        {
            Logger_debug("wal:%s", file.c_str());
            continue;
        }

        // ss table 文件
        size_t pos = file.find(EXT);
        if(std::string::npos != pos)
        {
            SsTable ss_table;
            if(false == ss_table.init(data_dir_  + "/" + file))
            {
                Logger_error("init ss table file failed:%s!", file.c_str());
                continue;
            }
            ss_tables_.push_back(ss_table);
            continue;
        }

        // wap 文件
        if(file == WAL)
        {
            Logger_debug("WAL:%s", file.c_str());
            continue;
        }
    }

    // 加载
    std::string path = data_dir + "/" + WAL;
    wal_file_ = ::fopen(path.c_str(), RW_MODE);
    if(0 == wal_file_)
    {
        Logger_error("create wal file failed!");
        abort();
    }

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
        // 交换内存表
        if(false == SwitchIndex())
        {
            return;
        }

        // 持久化
        if(false == StoreToSsTable())
        {
            return;
        }
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

        if(false == StoreToSsTable())
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
    SsTable ss_table;
    std::string path = base::CombineString("%s/%ld%s%", data_dir_.c_str(), "/", std::time(0), EXT);
    if(false == ss_table.init(path, part_size_, immutable_index_))
    {
        return false;
    }
    // 清理
    immutable_index_.clear();
    // 删除wal tmp
    std::string tmp_path = data_dir_ + "/" + WAL_TMP;
    base::FolderDelete(tmp_path);

    // 2. 添加到sstable列表头部
    ss_tables_.push_front(ss_table);

    return true;
}
//---------------------------------------------------------------------------
std::list<std::string> KvStore::GetSsTableFileList()
{
    std::list<std::string> res;
    DIR* dir = opendir(data_dir_.c_str());
    if(!dir)
    {
        return res;
    }

    dirent* ptr = readdir(dir); 
    while (ptr)
    {
        if(strcmp(ptr->d_name,".")==0 || strcmp(ptr->d_name,"..")==0)
        {
            ptr = readdir(dir);
            continue;
        }

        if(ptr->d_type == DT_REG)
        {
            res.push_back(ptr->d_name);
        }

        ptr = readdir(dir);
    }
    
    closedir(dir);

    // 按照时间排序（直接排序就好了）
    res.sort(std::greater<std::string>());
    return res;
}
//---------------------------------------------------------------------------

}//namespace lsm
//---------------------------------------------------------------------------
