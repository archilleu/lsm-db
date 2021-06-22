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
    RmCommand(const std::string& key);

    std::string ToString() const;

public:
    std::string get_key() { return key_; }

private:
    std::string key_;
};


}//namespace lsm
//---------------------------------------------------------------------------
#endif //LSM_RM_COMMAND_H_
