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
#include <cstdio>
#include <memory>

#include "./command/command.h"
#include "./ss_table.h"
//---------------------------------------------------------------------------
namespace lsm
{

class KvStore
{
public:
    KvStore(std::string data_dir, size_t store_threshold, size_t part_size);
    ~KvStore();

public:
    // 保存数据
    void Set(const std::string& key, const std::string& value);

    // 获取数据
    std::string Get(const std::string& key) const;

    // 删除数据
    void Rm(const std::string& key);

private:
    // 保存修改命令到WAL文件
    bool WriteToWalFile(const std::shared_ptr<Command>& command);

    // 切换内存表
    bool SwitchIndex();

    // 持久化到sstable
    bool StoreToSsTable();

public:
    static const char* EXT;     // 扩展名
    static const char* WAL;     // 日志文件
    static const char* RW_MODE; // 读写模式
    static const char* WAL_TMP; // 临时日志文件

private:
    // 内存表，使用有序的map，方便转储合并
    std::map<std::string, std::shared_ptr<Command>> index_;
    
    /**
     * 不可变内存表，用于持久化index_的时候暂存数据
     * 转储的时候写操作都在这个内存表里面
     */
    std::map<std::string, std::shared_ptr<Command>> immutable_index_;

    // ss-table列表
    std::list<SsTable> ss_tables_;

    // 数据目录
    std::string data_dir_;

    // FIXME: 优化锁，目前图简单用一把大锁
    std::mutex mutex_;

    // 持久化阈值
    size_t store_threshold_;

    // 数据分区大小
    size_t part_size_;

    // 暂存日志文件
    FILE* wal_file_;
};


}//namespace lsm
//---------------------------------------------------------------------------
#endif //LSM_KV_STORE_H_
