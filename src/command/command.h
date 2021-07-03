//---------------------------------------------------------------------------
#ifndef LSM_COMMAND_H_
#define LSM_COMMAND_H_
//---------------------------------------------------------------------------
#include <string>

#include "../../thirdpart/base/include/timestamp.h"

#include "command_type.h"
//---------------------------------------------------------------------------
namespace lsm
{

class Command
{
public:
    Command()
    {}

    Command(CommandType type, const std::string& key)
    {
        type_ = type;
        key_ = key;
        timestamp_ = base::Timestamp::Now();
    }
    virtual ~Command(){}


public:
    CommandType get_type() const { return type_; }

    std::string get_key() { return key_; }

    base::Timestamp get_timestamp() const { return timestamp_; }

private:
    // 命令类型
    CommandType type_;

    // key值
    std::string key_;


    //时间戳
    base::Timestamp timestamp_;
};


}//namespace lsm
//---------------------------------------------------------------------------
#endif //LSM_COMMAND_H_
