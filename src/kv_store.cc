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
const char* WAL = "wal";            // 扩展名;
const char* WAL_TMP = "wal_tmp";    // 扩展名
//---------------------------------------------------------------------------
KvStore::KvStore(const std::string& data_dir, size_t store_threshold, size_t part_size)
{
    data_dir_ = data_dir;
    wal_name_ = data_dir_ + "/" + WAL;
    wal_tmp_name_ = data_dir_ + "/" + WAL_TMP;
    store_threshold_ = store_threshold;
    part_size_ = part_size;

    return;
}
//---------------------------------------------------------------------------
KvStore::~KvStore()
{
    if(wal_fd_ > 0)
    {
        ::close(wal_fd_);
    }
    Logger_debug("dctor kv store");
}
//---------------------------------------------------------------------------
bool KvStore::Init()
{
    Logger_debug("ctor kv store->data path:%s, threshold:%ld, part_size:%ld", data_dir_.c_str(), store_threshold_, part_size_);

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
    std::list<std::string> files = GetSsTableFileList();
    for(std::string file : files)
    {
        // wap tmp 文件，持久化时候异常，需要还原异常的命令
        if(file == WAL_TMP)
        {
            if(false == ReadFromWalFile(data_dir_ + "/" + file))
            {
                Logger_error("restore from wal file failed:%s", file.c_str());
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
            if(false == ReadFromWalFile(data_dir_ + "/" + file))
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

    // FIXME: 细化锁
    std::lock_guard<std::mutex> guard(mutex_);

    // 1.写到wal文件
    WriteToWalFile(command);

    // 2.写到index
    index_[key] = command;

    // 3.如果超过阈值进行持久化
    if(index_.size() >= store_threshold_)
    {
        // 交换内存表
        if(false == SwitchIndex())
        {
            return false;
        }

        // 持久化
        if(false == StoreToSsTable())
        {
            return false;
        }
    }

    return true;
}
//---------------------------------------------------------------------------
std::string KvStore::Get(const std::string& key) const
{
    // 1.先从索引中查找
    std::shared_ptr<Command> result;
    auto it = index_.find(key);
    if(index_.end() != it)
    {
        result = it->second;
    }
    else
    {
        // 2.可能在持久化过程中, 尝试从不可变索引中查找
        if(!immutable_index_.empty())
        {
            it = immutable_index_.find(key);
            if(immutable_index_.end() != it)
            {
                result = it->second;
            }
        }
    }

    // 3.还找不到，尝试从ss table中查找
    if(!result)
    {
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
        return "";
    }

    // 找到
    Command* command = result.get();
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
bool KvStore::Rm(const std::string& key)
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
            return false;
        }

        if(false == StoreToSsTable())
        {
            return false;
        }

    }

    return true;
}
//---------------------------------------------------------------------------
bool KvStore::WriteToWalFile(const std::shared_ptr<Command>& command)
{
    std::string command_str = tools::CommandConvert::CommandToJsonStr(command);
    ssize_t size = command_str.size();
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
bool KvStore::SwitchIndex()
{
    // 1. 把数据转移到不可变内存表中
    assert(((void)"imutable_index not null", immutable_index_.empty()));
    index_.swap(immutable_index_);

    // 2. 切换wal文件
    ::close(wal_fd_);
    if(-1 == ::rename(wal_name_.c_str(), wal_tmp_name_.c_str()))
    {
        Logger_error("switch index failed:%s", tools::Logger::OsError(errno));
        return false;
    }

    // 3. 重新生成wal文件
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
    // 0. 检查是否需要合并ss-table
    if(NeddMergeSsTable())
    {
        MergeSsTableFile();
    }

    // 1. 按照时间戳生成sstable
    std::shared_ptr<SsTable> ss_table = std::make_shared<SsTable>();
    std::string path = CreateSsTableFilePath();
    if(false == ss_table->Init(path, part_size_, immutable_index_))
    {
        Logger_error("ss table init failed");
        return false;
    }
    // 清理
    immutable_index_.clear();
    // 删除wal tmp
    base::FileDelete(wal_tmp_name_);

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
bool KvStore::NeddMergeSsTable()
{
    if(ss_tables_.size() > 2)
    {
        return true;
    }

    return false;
}
//---------------------------------------------------------------------------
void KvStore::MergeSsTableFile()
{
    base::Thread thread(std::bind(&KvStore::OnMergeSsTable, this), "merge-sstable");
    thread.Start();
    thread.Join();
    return;
}
//---------------------------------------------------------------------------
void KvStore::OnMergeSsTable()
{
    Logger_debug("start merge ss-table!");

    std::shared_ptr<SsTable> ss_table = std::make_shared<SsTable>();
    std::string path = CreateSsTableFilePath();
    (void)path;
    Logger_debug("end merge ss-table!");
    return;
}
//---------------------------------------------------------------------------
}//namespace lsm
//---------------------------------------------------------------------------
