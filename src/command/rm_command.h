//---------------------------------------------------------------------------
#ifndef LSM_RM_COMMAND_H_
#define LSM_RM_COMMAND_H_
//---------------------------------------------------------------------------
#include "abstract_command.h"
//---------------------------------------------------------------------------
namespace lsm
{

class RmCommand : public AbstractCommand
{
public:
    RmCommand(const std::string& key)
    :   AbstractCommand(CommandType::RM)
    {
        key_ = key;
    }

public:
    std::string get_key() { return key_; }


private:
    std::string key_;
};


}//namespace lsm
//---------------------------------------------------------------------------
#endif //LSM_RM_COMMAND_H_
