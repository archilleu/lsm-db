//---------------------------------------------------------------------------
#ifndef LSM_RM_COMMAND_H_
#define LSM_RM_COMMAND_H_
//---------------------------------------------------------------------------
#include "../../thirdpart/base/include/json/json.h"

#include "command.h"
//---------------------------------------------------------------------------
namespace lsm
{

//---------------------------------------------------------------------------
using namespace base::json;
//---------------------------------------------------------------------------
class RmCommand : public Command
{
public:
    RmCommand(const std::string& key)
    :   Command(CommandType::RM, key)
    {

    }
};


}//namespace lsm
//---------------------------------------------------------------------------
#endif //LSM_RM_COMMAND_H_
