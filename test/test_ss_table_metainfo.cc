//---------------------------------------------------------------------------
#include <iostream>
#include <assert.h>

#include "../src/ss_table_metainfo.h"
//---------------------------------------------------------------------------
using namespace lsm;
//---------------------------------------------------------------------------
int main(int, char**)
{
    const char* path = "./lsm.dump";
    // dump
    {
    FILE* file = ::fopen(path, "wb");
    SsTableMetainfo metainfo = SsTableMetainfo();
    metainfo.set_version(1l);
    metainfo.set_data_start(2l);
    metainfo.set_data_len(3l);
    metainfo.set_index_start(4l);
    metainfo.set_index_len(5l);
    metainfo.set_part_size(6l);
    if(false == metainfo.Dump(file))
    {
    assert(((void)"dump failure", false));
    }
        
    ::fclose(file);
    }

    // undump
    {
    FILE* file = ::fopen(path, "rb");
    SsTableMetainfo metainfo = SsTableMetainfo();
    if(false == SsTableMetainfo::UnDump(&metainfo, file))
    {
        assert(((void)"undump failure", false));
    }

    assert(metainfo.get_version() == 1l);
    assert(metainfo.get_data_start() == 2l);
    assert(metainfo.get_data_len() == 3l);
    assert(metainfo.get_index_start() == 4l);
    assert(metainfo.get_index_len() == 5l);
    assert(metainfo.get_part_size() == 6l);
    }

    return 0;
}
