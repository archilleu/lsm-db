//---------------------------------------------------------------------------
#ifndef LSM_COMMAND_TYPE_H_
#define LSM_COMMAND_TYPE_H_
//---------------------------------------------------------------------------
#include <string>
//---------------------------------------------------------------------------
namespace lsm
{

enum class CommandType
{
    // 设置命令
    SET,

    // 删除命令
    RM
};


}//namespace lsm
//---------------------------------------------------------------------------
#endif //LSM_COMMAND_TYPE_H_