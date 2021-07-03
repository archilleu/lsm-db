//---------------------------------------------------------------------------
#ifndef LSM_KV_STORE_H_
#define LSM_KV_STORE_H_
//---------------------------------------------------------------------------
/**
 * kv db implement base on lsm-tree
 */
//---------------------------------------------------------------------------
#include <string>
#include <map>
#include <list>
#include <mutex>
#include <memory>

#include "../thirdpart/base/include/logger.h"

#include "./command/command.h"
//---------------------------------------------------------------------------
class SsTable;
//---------------------------------------------------------------------------
namespace lsm
{

class KvStore
{
public:
    KvStore(const std::string& data_dir, size_t store_threshold, size_t part_size);
    ~KvStore();

public:
    // 初始化
    bool Init();

    void SetLogger(std::shared_ptr<base::Logger> logger);

    // 保存数据
    bool Set(const std::string& key, const std::string& value);

    // 获取数据
    std::string Get(const std::string& key) const;

    // 删除数据
    bool Rm(const std::string& key);

private:
    // 保存命令到WAL文件
    bool WriteToWalFile(const std::shared_ptr<Command>& command);

    // 切换内存表
    bool SwitchIndex();

    // 持久化到sstable
    bool StoreToSsTable();

    // 获取ss table 文件名列表
    std::list<std::string> GetSsTableFileList();

    // 创建ss table文件路径
    std::string CreateSsTableFilePath();

    // 还原wal文件命令
    bool ReadFromWalFile(const std::string& path);

    // 合并ss-table线程
    bool NeddMergeSsTable();
    void MergeSsTableFile();
    void OnMergeSsTable();

private:
    using IndexMap = std::map<std::string, std::shared_ptr<Command>>;

    // 内存表，使用有序的map，方便转储合并
    IndexMap index_;
    
    /**
     * 不可变内存表，用于持久化index_的时候暂存数据
     * 转储的时候写操作都在这个内存表里面
     */
    IndexMap immutable_index_;

    // ss-table列表
    std::list<std::shared_ptr<SsTable>> ss_tables_;

    // 数据目录
    std::string data_dir_;
    std::string wal_name_;
    std::string wal_tmp_name_;

    // FIXME: 优化锁，目前图简单用一把大锁
    std::mutex mutex_;

    // 持久化阈值
    size_t store_threshold_;

    // 数据分区大小
    size_t part_size_;

    // 暂存日志文件
    int wal_fd_;
};

}//namespace lsm
//---------------------------------------------------------------------------
#endif //LSM_KV_STORE_H_
