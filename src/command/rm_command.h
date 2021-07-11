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
    :   RmCommand(key, base::Timestamp::Now())
    {
    }

    RmCommand(const std::string& key, base::Timestamp timestamp)
    :   Command(CommandType::RM, key, timestamp)
    {
    }
};


}//namespace lsm
//---------------------------------------------------------------------------
#endif //LSM_RM_COMMAND_H_
