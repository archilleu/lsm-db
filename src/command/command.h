//---------------------------------------------------------------------------
#ifndef LSM_COMMAND_H_
#define LSM_COMMAND_H_
//---------------------------------------------------------------------------
#include <string>
//---------------------------------------------------------------------------
namespace lsm
{

class Command
{
public:
    virtual std::string get_key() =0;

};


}//namespace lsm
//---------------------------------------------------------------------------
#endif //LSM_COMMAND_H_