//---------------------------------------------------------------------------
#include <iostream>
#include <assert.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <cstring>

#include "../src/ss_table_metainfo.h"
//---------------------------------------------------------------------------
using namespace lsm;
__thread char t_error_buf[512];
//---------------------------------------------------------------------------
int main(int, char**)
{
    const char* path = "./lsm.dump";
    // dump
    {
    int fd = ::open(path, O_RDWR|O_CREAT, 0770);
    if(fd == -1)
    {
        assert(((void)"open", false));
    }
    SsTableMetainfo metainfo = SsTableMetainfo();
    metainfo.set_version(1l);
    metainfo.set_data_start(2l);
    metainfo.set_data_len(3l);
    metainfo.set_index_start(4l);
    metainfo.set_index_len(5l);
    metainfo.set_part_size(6l);
    if(false == metainfo.WriteToFile(fd))
    {
        assert(((void)"dump failure", false));
    }
    ::close(fd);
    }

    // undump
    {
    int fd = ::open(path, O_RDONLY);
    SsTableMetainfo metainfo = SsTableMetainfo();
    if(false == SsTableMetainfo::ReadFromFile(&metainfo, fd))
    {
        assert(((void)"undump failure", false));
    }
    ::close(fd);

    assert(metainfo.get_version() == 1l);
    assert(metainfo.get_data_start() == 2l);
    assert(metainfo.get_data_len() == 3l);
    assert(metainfo.get_index_start() == 4l);
    assert(metainfo.get_index_len() == 5l);
    assert(metainfo.get_part_size() == 6l);
    }

    return 0;
}
