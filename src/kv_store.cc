//---------------------------------------------------------------------------
#include <ctime>
#include <cstring>
#include <cstdlib>
#include <sys/time.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <dirent.h>

#include "../thirdpart/base/include/function.h"

#include "ss_table.h"
#include "kv_store.h"
#include "command/set_command.h"
#include "command/rm_command.h"
#include "tools/logger.h"
#include "tools/command_convert.h"
//---------------------------------------------------------------------------
namespace lsm
{

//---------------------------------------------------------------------------
const char* EXT = ".table";         // 扩展名
const char* WAL = "wal";            // 操作日志文件
const char* WAL_TMP = "wal_tmp";    // 临时操作日志文件
const char* NULL_VALUE = "";        // 空值
//---------------------------------------------------------------------------
class CommandSsTablePair
{
public:
    CommandSsTablePair(const std::shared_ptr<Command>& cmd, const std::shared_ptr<SsTable>& table)
    {
        command = cmd;
        ss_table = table;
    }

    std::shared_ptr<Command> command;
    std::shared_ptr<SsTable> ss_table;

    /**
     * set容器在判定已有元素a和新插入元素b是否相等时，是这么做的：
     * 1）将a作为左操作数，b作为有操作数，调用比较函数，并返回比较值  
     * 2）将b作为左操作数，a作为有操作数，再调用一次比较函数，并返回比较值。
     * 如果1、2两步的返回值都是false，则认为a、b是相等的，则b不会被插入set容器中；
     * 如果1、2两步的返回值都是true，则可能发生未知行为，因此，记住一个准则：永远让比较函数对相同元素返回false。
    */
    bool operator<(const CommandSsTablePair& other) const
    {
        return command->get_key() < other.command->get_key();
    }
};
//---------------------------------------------------------------------------
KvStore::KvStore(const std::string& data_dir, size_t store_threshold, size_t part_size)
{
    data_dir_ = data_dir;
    wal_name_ = data_dir_ + "/" + WAL;
    wal_tmp_name_ = data_dir_ + "/" + WAL_TMP;
    store_threshold_ = store_threshold;
    part_size_ = part_size;
    is_maintain_ = false;

    return;
}
//---------------------------------------------------------------------------
KvStore::~KvStore()
{
    if(wal_fd_ > 0)
    {
        ::close(wal_fd_);
    }

    while(is_maintain_.load(std::memory_order_acquire))
    {
        ::sleep(1);
        Logger_info("waiting for store or merge sstable done!");
    }

    Logger_debug("dctor kv store");
}
//---------------------------------------------------------------------------
bool KvStore::Init()
{
    Logger_debug("initializing kv store:data path:%s, threshold:%ld, part_size:%ld", data_dir_.c_str(), store_threshold_, part_size_);

    // 生成ss table目录
    if(!base::FolderExist(data_dir_))
    {
        if(false == base::FolderCreate(data_dir_, true))
        {
            Logger_error("create data folder failed!");
            return false;
        }
    }

    // 加载ss table
    std::list<std::string> files = GetKvStoreFileList();
    for(std::string file : files)
    {
        // wap tmp 文件，持久化时候异常，需要还原异常的命令
        if(file == WAL_TMP)
        {
            if(false == ReadFromWalFile(wal_tmp_name_))
            {
                Logger_error("restore from wal file failed:%s", file.c_str());
            }
            else
            {
                base::FileDelete(wal_tmp_name_);
            }
            continue;
        }

        // ss table 文件
        size_t pos = file.find(EXT);
        if(std::string::npos != pos)
        {
            std::shared_ptr<SsTable> ss_table = std::make_shared<SsTable>();
            if(false == ss_table->Init(data_dir_  + "/" + file))
            {
                Logger_error("init ss table file failed:%s", file.c_str());
                continue;
            }
            ss_tables_.push_back(ss_table);
            continue;
        }

        // wap 文件
        if(file == WAL)
        {
            if(false == ReadFromWalFile(wal_name_))
            {
                Logger_error("restore from wal file failed:%s", file.c_str());
            }
            continue;
        }
    }

    // 打开wal文件
    wal_fd_ = ::open(wal_name_.c_str(), O_CREAT|O_WRONLY|O_APPEND, 0660);
    if(wal_fd_ < 0)
    {
        Logger_error("create wal file failed:%s", tools::Logger::OsError(errno));
    }

    Logger_debug("initializd kv store:data path:%s, threshold:%ld, part_size:%ld", data_dir_.c_str(), store_threshold_, part_size_);
    return true;
}
//---------------------------------------------------------------------------
void KvStore::SetLogger(std::shared_ptr<base::Logger> logger)
{
    tools::g_logger.set_logger(logger);
}
//---------------------------------------------------------------------------
bool KvStore::Set(const std::string& key, const std::string& value)
{
    std::shared_ptr<SetCommand> command = std::make_shared<SetCommand>(key, value);
    return SetOrRm(command);
}
//---------------------------------------------------------------------------
std::string KvStore::Get(const std::string& key)
{
    // 1.先从索引中查找
    std::shared_ptr<Command> result;
    {
        std::lock_guard<std::mutex> guard(index_mutex_);
        auto it = index_.find(key);
        if(index_.end() != it)
        {
            result = it->second;
        }
    }

    // 2.可能在持久化过程中, 尝试从不可变索引中查找
    {
        if(!result)
        {
            std::lock_guard<std::mutex> guard(immutable_index_mutex_);
            if(!immutable_index_.empty())
            {
                auto it = immutable_index_.find(key);
                if(immutable_index_.end() != it)
                {
                    result = it->second;
                }
            }
        }
    }

    // 3.还找不到，尝试从ss table中查找
    if(!result)
    {
        // 查询的时候锁定ss_tables_，如果在合并ss_tables过程中，锁时间很短，不影响查询性能
        // TODO:复制一份ss_tables?复制过程比较耗时，还不如直接锁
        std::lock_guard<std::mutex> guard(ss_tables_mutex_);
        for(auto& ss_table : ss_tables_)
        {
            result = ss_table->Query(key);
            if(result)
            {
                break;
            }
        }
    }

    // 4.找不到
    if(!result)
    {
        return NULL_VALUE;
    }

    // 找到
    Command* command = result.get();
    if(command->get_type() == CommandType::SET)
    {
        SetCommand* set = static_cast<SetCommand*>(command);
        return set->get_value();
    }
    else
    {
        // rm command, null value
        return NULL_VALUE;
    }
}
//---------------------------------------------------------------------------
bool KvStore::Rm(const std::string& key)
{
    std::shared_ptr<RmCommand> command = std::make_shared<RmCommand>(key);
    return SetOrRm(command);
}
//---------------------------------------------------------------------------
bool KvStore::SetOrRm(std::shared_ptr<Command> command)
{
    // 1.写到wal文件
    if(false == WriteToWalFile(command))
    {
        return false;
    }

    {
        // 2.写到index
        std::lock_guard<std::mutex> guard(index_mutex_);
        index_[command->get_key()] = command;

        // 3.如果不超过阈值直接返回
        if(index_.size() < store_threshold_)
        {
            return true;
        }

        // 4.sstable维护中，直接返回，不支持多线程维护sstable
        if(is_maintain_.load(std::memory_order_acquire))
        {
            Logger_warn("sstable maintainning");
            return true;
        }
        else
        {
            is_maintain_ = true;
        }


        // 5.交换内存索引
        std::lock_guard<std::mutex> immutable_guard(immutable_index_mutex_);
        index_.swap(immutable_index_);
    }

    //重新创建wal文件
    CreateNewWalFile();

    // 开始维护sstable线程
    std::thread t(&KvStore::OnMaintainSsTable, this);
    t.detach();

    return true;
}
//---------------------------------------------------------------------------
bool KvStore::WriteToWalFile(const std::shared_ptr<Command>& command)
{
    std::string command_str = tools::CommandConvert::CommandToJsonStr(command);
    ssize_t size = command_str.size();

    std::lock_guard<std::mutex> guard(wal_mutex_);
    if(::write(wal_fd_, &size, sizeof(size)) != sizeof(size))
    {
        return false;
    }
    if(::write(wal_fd_, command_str.c_str(), command_str.length()) != size)
    {
        return false;
    }
    ::fdatasync(wal_fd_);

    return true;
}
//---------------------------------------------------------------------------
void KvStore::OnMaintainSsTable()
{
    // 是否需要合并ss-table
    MergeSsTable();

    // 持久化sstable
    StoreToSsTable();

    is_maintain_ = false;
}
//---------------------------------------------------------------------------
void KvStore::MergeSsTable()
{
    // 判断是否需要合并
    {
        std::lock_guard<std::mutex> guard(ss_tables_mutex_);
        if(ss_tables_.size() < 2)
        {
            return;
        }
    }

    Logger_debug("start merge ss-table!");

    // 获取当前所有ss_table
    std::vector<std::shared_ptr<SsTable>> ss_tables = GetMergeSsTables();

    // 生成merge sstable
    std::shared_ptr<SsTable> merge_ss_table = std::make_shared<SsTable>();
    std::string path = CreateSsTableFilePath();

    // 开始合并
    if(false == merge_ss_table->StartMerge(path, part_size_))
    {
        Logger_error("start merge failed");
        return;
    }

    MergeSstables(ss_tables, merge_ss_table);
    
    if(false == merge_ss_table->EndMerge())
    {
        Logger_error("end merge failed");
        return;
    }
    
    // 合并完成后移除合并过的sstable
    {
        std::lock_guard<std::mutex> guard(ss_tables_mutex_);
        for(auto it=ss_tables.begin(); it!=ss_tables.end(); it++)
        {
            for(auto iter=ss_tables_.begin(); iter!=ss_tables_.end();)
            {
                if((*it)->get_path() == (*iter)->get_path())
                {
                    ss_tables_.erase(iter++);
                    base::FileDelete((*it)->get_path());
                }
                else
                {
                    ++iter;
                }
            }
        }
        ss_tables_.push_back(merge_ss_table);
    }

    Logger_debug("end merge ss-table!");
    return;
}
//---------------------------------------------------------------------------
bool KvStore::CreateNewWalFile()
{
    // 切换wal文件
    std::lock_guard<std::mutex> guard(wal_mutex_);
    ::close(wal_fd_);
    if(-1 == ::rename(wal_name_.c_str(), wal_tmp_name_.c_str()))
    {
        Logger_error("switch index failed:%s", tools::Logger::OsError(errno));
        return false;
    }

    // 重新生成wal文件
    wal_fd_ = ::open(wal_name_.c_str(), O_CREAT|O_WRONLY|O_APPEND, 0660);
    if(wal_fd_ < 0)
    {
        Logger_error("create wal file failed:%s", tools::Logger::OsError(errno));
        return false;
    }

    return true;
}
//---------------------------------------------------------------------------
bool KvStore::StoreToSsTable()
{
    // 1. 按照时间戳生成sstable
    std::shared_ptr<SsTable> ss_table = std::make_shared<SsTable>();
    std::string path = CreateSsTableFilePath();
    if(false == ss_table->Init(path, part_size_, immutable_index_))
    {
        Logger_error("ss table init failed");
        return false;
    }

    // 2. 添加到sstable列表头部
    {
        std::lock_guard<std::mutex> guard(ss_tables_mutex_);
        ss_tables_.push_front(ss_table);
    }

    // 3. 清理
    {
        std::lock_guard<std::mutex> guard(immutable_index_mutex_);
        immutable_index_.clear();
    }

    // 4. 删除wal tmp
    base::FileDelete(wal_tmp_name_);

    return true;
}
//---------------------------------------------------------------------------
std::list<std::string> KvStore::GetKvStoreFileList()
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
        if(strcmp(ptr->d_name, ".")==0 || strcmp(ptr->d_name, "..")==0 || strstr(ptr->d_name, EXT)==0)
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
std::string KvStore::CreateSsTableFilePath()
{
    struct timeval t;
    gettimeofday(&t, 0);
    long time = static_cast<long>(static_cast<long>(t.tv_sec * 1000 * 1000) + t.tv_usec);
    std::string path = base::CombineString("%s/%ld%s", data_dir_.c_str(), time, EXT);
    return path;
}
//---------------------------------------------------------------------------
bool KvStore::ReadFromWalFile(const std::string& path)
{
    int fd = ::open(path.c_str(), O_RDONLY);
    if(fd < 0)
    {
        Logger_error("read wal file failed:%s", path.c_str());
        return false;
    }

    // 读取每一条command
    ssize_t size = 0;
    std::string command_str;
    while (::read(fd, &size, sizeof(size)) == sizeof(size))
    {
        command_str.resize(size);
        if(::read(fd, const_cast<char*>(command_str.data()), size) != size)
        {
            Logger_error("read wal file failed:%s", tools::Logger::OsError(errno));
            ::close(fd);
            return false;
        }

        // 插入命令
        auto command = tools::CommandConvert::JsonStrToCommand(command_str);
        if(nullptr == command)
        {
            ::close(fd);
            Logger_error("read wal file command failed:%s,%s", path.c_str(), command_str.c_str());
            return false;
        }
        index_[command->get_key()] = command;
    }
    
    ::close(fd);
    return true;
}
//---------------------------------------------------------------------------
std::vector<std::shared_ptr<SsTable>> KvStore::GetMergeSsTables()
{
    // 遍历ss_table
    std::vector<std::shared_ptr<SsTable>> ss_tables;
    std::list<std::string> files = GetSsTableFileList();
    for(std::string file : files)
    {
        std::shared_ptr<SsTable> table = std::make_shared<SsTable>();
        if(false == table->Init(data_dir_  + "/" + file))
        {
            Logger_info("init ss table failed:%s", file.c_str());
            continue;
        }
        if(false == table->Begin())
        {
            Logger_info("traverse sstable failed:%s", file.c_str());
            continue;
        }

        ss_tables.push_back(table);
    }

    return ss_tables;
}
//---------------------------------------------------------------------------
void TrimList(std::list<CommandSsTablePair>& list)
{
    if(list.size() > 1)
    {
        auto first = list.begin();
        auto second = ++list.begin();
        if(first->command->get_key() == second->command->get_key())
        {
            std::list<CommandSsTablePair>::const_iterator iter;
            if(first->command->get_timestamp() > second->command->get_timestamp())
            {
                iter = second;
            }
            else
            {
                iter = first;
            }
            if(iter->ss_table->HasNext())
            {
                CommandSsTablePair pair(iter->ss_table->Next(), iter->ss_table);
                list.erase(iter);
                list.push_back(pair);
            }
            else
            {
                list.erase(iter);
            }

            list.sort();
        }
    }
}
//---------------------------------------------------------------------------
bool KvStore::MergeSstables(const std::vector<std::shared_ptr<SsTable>>& ss_tables, const std::shared_ptr<SsTable>& merge_ss_table)
{
    // 初始化
    std::list<CommandSsTablePair> list;
    for(auto& table : ss_tables)
    {
        if(true == table->HasNext())
        {
            auto command = table->Next();
            if(command)
            {
                list.push_back(CommandSsTablePair(command, table));
            }
        }
    }
    list.sort();

    /**
     * 归并排序
     * https://juejin.cn/post/6844903762621005837
     */

    // 取最小元素
    while (!list.empty())
    {
        TrimList(list);
        auto min = list.begin();
        if(min->ss_table->HasNext())
        {
            auto next_command = min->ss_table->Next();
            auto& cur_command = min->command;

            // 不可能相等
            // if(command->get_key() == next_command->get_key()){}

            merge_ss_table->Merge(cur_command);
            list.erase(min);
            list.push_back(CommandSsTablePair(next_command, min->ss_table));
            list.sort();
        }
        else
        {
            merge_ss_table->Merge(min->command);
            list.erase(min);
        }
    }
    
    return true;
}
//---------------------------------------------------------------------------
}//namespace lsm
//---------------------------------------------------------------------------