//---------------------------------------------------------------------------
#include "kv_store.h"
#include "command/set_command.h"
#include "command/rm_command.h"
#include "./tools/logger.h"
//---------------------------------------------------------------------------
namespace lsm
{

void KvStore::Set(const std::string& key, const std::string& value)
{
    SetCommand command = SetCommand(key, value);
    (void)command;
    return;
}
//---------------------------------------------------------------------------
std::string KvStore::Get(const std::string& key) const
{
    return key;
}
//---------------------------------------------------------------------------
void KvStore::Rm(const std::string& key)
{
    RmCommand command = RmCommand(key);
    (void)command;
    return;
}
//---------------------------------------------------------------------------

}//namespace lsm
//---------------------------------------------------------------------------
