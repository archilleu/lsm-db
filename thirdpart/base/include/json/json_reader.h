//---------------------------------------------------------------------------
#ifndef BASE_JSON_JSON_READER_H_
#define BASE_JSON_JSON_READER_H_
//---------------------------------------------------------------------------
#include <string>
#include <stack>
#include <memory>
#include "value.h"
//---------------------------------------------------------------------------
namespace base
{

namespace json
{

class TokenReader;

class JsonReader
{
public:
    JsonReader();
    ~JsonReader();

    //解析字符串,以\0作为结束符
    bool Parse(const std::string& str, Value& root);
    bool Parse(std::string&& str, Value& root);
    bool Parse(const char* str, Value& root);
    
    //解析内存数据,可以不用\0为结束符
    bool Parse(const char* str, size_t len, Value& root);

    //解析JSON格式文件
    bool ParseFile(const std::string& path, Value& root);
    bool ParseFile(const char* path, Value& root);

private:
    bool _Parse(Value& root);

    //当前是否有对应的状态
    bool HasStatus(int status) { return (cur_status_ & status); }

    Value ReadNumber();

private:
    bool CaseStatusObjectBegin();
    bool CaseStatusObjectKey();
    bool CaseStatusObjectValue(int type);
    bool CaseStatusObjectEnd();
    bool CaseStatusArrayBegin();
    bool CaseStatusArrayValue(int type);
    bool CaseStatusArrayEnd();
    bool CaseStatusSepColon();
    bool CaseStatusSepComma();
    bool CaseStatusDocumentEnd();

private:
    std::shared_ptr<TokenReader> token_reader_;
    std::stack<Value> parse_stack_;
    int cur_status_;    //当前解析状态,即下一个期待的字符类型

private:
    //解析过程中的状态(期待的下一个字符类型)
    static const int kEXP_STATUS_OBJECT_BEGIN = 0x0001;     //期待对象
    static const int kEXP_STATUS_OBJECT_KEY   = 0x0002;     //期待对象key值
    static const int kEXP_STATUS_OBJECT_VALUE = 0x0004;     //期待对象Value值
    static const int kEXP_STATUS_OBJECT_END   = 0x0008;     //期待对象结束
    static const int kEXP_STATUS_ARRAY_BEGIN  = 0x0010;     //期待数组开始
    static const int kEXP_STATUS_ARRAY_VALUE  = 0x0020;     //期待数组Value
    static const int kEXP_STATUS_ARRAY_END    = 0x0040;     //期待数组结束
    static const int kEXP_STATUS_SEP_COLON    = 0x0080;     //期待:
    static const int kEXP_STATUS_SEP_COMMA    = 0x0100;     //期待,
    static const int kEXP_STATUS_DOCUMENT_END = 0x0200;     //期待JSON解析结束
};

}//namespace json

}//namespace base
//---------------------------------------------------------------------------
#endif //BASE_JSON_JSON_READER_H_
