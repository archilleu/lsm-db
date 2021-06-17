//---------------------------------------------------------------------------
#ifndef LSM_ABSTRACT_COMMAND_H_
#define LSM_ABSTRACT_COMMAND_H_
//---------------------------------------------------------------------------
#include "command.h"
#include "command_type.h"
//---------------------------------------------------------------------------
namespace lsm
{

class AbstractCommand : public Command
{
public:
    AbstractCommand(CommandType type)
    {
        type_ = type;
    }

private:
    CommandType type_;
};


}//namespace lsm
//---------------------------------------------------------------------------
#endif //LSM_ABSTRACT_COMMAND_H_
