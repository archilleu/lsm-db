//---------------------------------------------------------------------------
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "./tools/logger.h"

#include "ss_table_metainfo.h"
//---------------------------------------------------------------------------
namespace lsm
{

//---------------------------------------------------------------------------
int SsTableMetainfo::METAINFO_SIZE = sizeof(SsTableMetainfo);
//---------------------------------------------------------------------------
SsTableMetainfo::SsTableMetainfo()
{
    version_ = 0;

    data_start_ = 0;
    data_len_ = 0;

    index_start_ = 0;
    index_len_ = 0;

    part_size_ =0;
}
//---------------------------------------------------------------------------
bool SsTableMetainfo::WriteToFile(int fd)
{
    if(false == WriteField(part_size_, fd))   return false;
    if(false == WriteField(data_start_, fd))  return false;
    if(false == WriteField(data_len_, fd))    return false;
    if(false == WriteField(index_start_, fd)) return false;
    if(false == WriteField(index_len_, fd))   return false;
    if(false == WriteField(version_, fd))     return false;

    return true;
}
//---------------------------------------------------------------------------
bool SsTableMetainfo::ReadFromFile(SsTableMetainfo* metainfo, int fd)
{
    // read from end of fd
    if(-1 == ::lseek(fd, -METAINFO_SIZE, SEEK_END))   return false;
    if(false == ReadField(&metainfo->part_size_, fd))   return false;
    if(false == ReadField(&metainfo->data_start_, fd))  return false;
    if(false == ReadField(&metainfo->data_len_, fd))    return false;
    if(false == ReadField(&metainfo->index_start_, fd)) return false;
    if(false == ReadField(&metainfo->index_len_, fd))   return false;
    if(false == ReadField(&metainfo->version_, fd))     return false;

    return true;
}
//---------------------------------------------------------------------------
bool SsTableMetainfo::WriteField(long field, int fd)
{
    if(sizeof(long) != ::write(fd, static_cast<void*>(&field), sizeof(long)))
    {
        Logger_error("%s", tools::Logger::OsError(errno));
        return false;
    }

    return true;
}
//---------------------------------------------------------------------------
bool SsTableMetainfo::ReadField(long* field, int fd)
{
    if(sizeof(long) != ::read(fd, field, sizeof(long)))
    {
        Logger_error("%s", tools::Logger::OsError(errno));
        return false;
    }

    return true;
}
//---------------------------------------------------------------------------

}//namespace lsm
//---------------------------------------------------------------------------
