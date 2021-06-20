//---------------------------------------------------------------------------
#include "ss_table_metainfo.h"
//---------------------------------------------------------------------------
namespace lsm
{

//---------------------------------------------------------------------------
long SsTableMetainfo::METAINFO_SIZE = sizeof(SsTableMetainfo);
//---------------------------------------------------------------------------
bool SsTableMetainfo::Dump(FILE* file)
{
    if(false == WriteField(part_size_, file))   return false;
    if(false == WriteField(data_start_, file))  return false;
    if(false == WriteField(data_len_, file))    return false;
    if(false == WriteField(index_start_, file)) return false;
    if(false == WriteField(index_len_, file))   return false;
    if(false == WriteField(version_, file))     return false;

    ::fflush_unlocked(file);
    return true;
}
//---------------------------------------------------------------------------
bool SsTableMetainfo::UnDump(SsTableMetainfo* metainfo, FILE* file)
{
    // read from end of file
    ::fseek(file, -(sizeof(long)*6), SEEK_END);
    if(false == ReadField(&metainfo->part_size_, file))     return false;
    if(false == ReadField(&metainfo->data_start_, file))    return false;
    if(false == ReadField(&metainfo->data_len_, file))      return false;
    if(false == ReadField(&metainfo->index_start_, file))   return false;
    if(false == ReadField(&metainfo->index_len_, file))     return false;
    if(false == ReadField(&metainfo->version_, file))       return false;

    return true;
}
//---------------------------------------------------------------------------
bool SsTableMetainfo::WriteField(long field, FILE* file)
{
    size_t wlen = ::fwrite_unlocked(static_cast<void*>(&field), 1, sizeof(long), file);
    if(wlen != sizeof(long))
        return false;

    return true;
}
//---------------------------------------------------------------------------
bool SsTableMetainfo::ReadField(long* field, FILE* file)
{
    size_t rlen = fread_unlocked(field, 1, sizeof(long), file);
    if(rlen != sizeof(long))
        return false;

    return true;
}
//---------------------------------------------------------------------------

}//namespace lsm
//---------------------------------------------------------------------------
