//---------------------------------------------------------------------------
#ifndef LSM_SS_TABLE_METAINFO_H_
#define LSM_SS_TABLE_METAINFO_H_
//---------------------------------------------------------------------------
/**
 * ss-table file metainfo
 * |----0->(data_len-1)----|-----data_len->(index_len-1)--|----metainfo------|
 * |--------data-----------|-----------index--------------|----metainfo------|
 */
//---------------------------------------------------------------------------
#include <stdio.h>
//---------------------------------------------------------------------------
namespace lsm
{

class SsTableMetainfo
{
public:
    bool Dump(FILE* file);
    static bool UnDump(SsTableMetainfo* metainfo, FILE* file);

public:
    long get_version() const { return version_; }
    long get_data_start() const { return data_start_; }
    long get_data_len() const { return data_len_; }
    long get_index_start() const { return index_start_; }
    long get_index_len() const { return index_len_; }
    long get_part_size() const { return part_size_; }

    void set_version(long version) { version_ = version; }
    void set_data_start(long data_start) { data_start_ = data_start; }
    void set_data_len(long data_len) { data_len_ = data_len; }
    void set_index_start(long index_start) { index_start_ = index_start; }
    void set_index_len(long index_len) { index_len_ = index_len; }
    void set_part_size(long part_size) { part_size_ = part_size; }

public:
    static long METAINFO_SIZE;

private:
    bool WriteField(long field, FILE* file);
    static bool ReadField(long* field, FILE* file);

private:
    long version_;

    long data_start_;
    long data_len_;

    long index_start_;
    long index_len_;

    long part_size_;
};


}//namespace lsm
//---------------------------------------------------------------------------
#endif //LSM_SS_TABLE_METAINFO_H_
