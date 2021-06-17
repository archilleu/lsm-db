//---------------------------------------------------------------------------
#ifndef BASE_JSON_VALUE_H_
#define BASE_JSON_VALUE_H_
//---------------------------------------------------------------------------
#include <inttypes.h>
#include <vector>
#include <map>
#include <string>
#include <assert.h>
//---------------------------------------------------------------------------
/*
 * 类型不对会抛出异常
 */
//---------------------------------------------------------------------------
namespace base
{

namespace json
{

//---------------------------------------------------------------------------
class type_error : public std::logic_error
{
public:
    type_error(const char* msg="type error")
    :   std::logic_error(msg)
    {
    }
    virtual ~type_error(){}
};
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
class Value
{
public:
    //Value的类型
    enum ValueType
    {
        Object,     //对象类型
        Array,      //数组类型
        String,     //字符串对象
        Int,        //有符号整形
        UInt,       //无符号整形
        Real,       //浮点型
        Number,     //数字类型(Int,UInt,Real)
        Boolean,    //布伦
        Null,       //空
        Key         //key，供解析用
    };

    //类型定义
    using ArrayValue        = std::vector<Value>;
    using ObjectValue       = std::map<std::string, Value>;
    using ArrayValueIter    = ArrayValue::const_iterator;
    using ObjectValueIter   = ObjectValue::const_iterator;

public:
    Value(ValueType type=ValueType::Null);
    Value(const std::string& value);
    Value(std::string&& value);
    Value(const char* value);
    Value(int value);
    Value(int64_t value);
    Value(unsigned int value);
    Value(uint64_t value);
    Value(double value);
    Value(bool value);
    Value(const Value& other);
    Value(Value&& other);
    Value& operator=(const Value& other);
    Value& operator=(Value&& other);
    ~Value();

    void Swap(Value& other);

public:
    ValueType type() const { return type_; }

    void set_int(int64_t value);
    void set_uint(uint64_t value);
    void set_boolean(bool value);
    void set_double(double value);
    void set_str(const char* str);
    void set_str(const std::string& str);
    void set_str(std::string&& str);
    void set_key(const char* str);
    void set_key(const std::string& str);
    void set_key(std::string&& str);

    bool IsInt() const;
    bool IsUInt() const;
    bool IsBoolean() const;
    bool IsReal() const;
    bool IsString() const;
    bool IsArray() const;
    bool IsObject() const;
    bool IsNull() const;
    bool IsKey() const;

    int AsInt() const;
    int64_t AsInt64() const;
    unsigned int AsUInt() const;
    uint64_t AsUInt64() const;
    bool AsBoolean() const;
    float AsFloat() const;
    double AsDouble() const;
    std::string& AsString();
    const std::string& AsString() const;
    std::string& AsKey();
    const std::string& AsKey() const;

    size_t Size() const;
    bool Empty() const;

    //ObjectValue
    //如果不存在则添加一个空值
    Value& operator[](const char* key);
    Value& operator[](const std::string& key);
    Value& operator[](const std::string&& key);

    //访问一个值，如果不存在返回NullValue
    const Value& operator[](const char* key) const;
    const Value& operator[](const std::string& key) const;

    //访问一个值，如果不存在返回NullValue
    const Value& ObjectGet(const std::string& key) const;
    const Value& ObjectGet(const char* key) const;

    bool ObjectDel(const std::string& key);
    bool ObjectDel(const char* key);

    ObjectValueIter ObjectIterBegin() const { return value_.object_->begin(); }
    ObjectValueIter ObjectIterEnd() const { return value_.object_->end(); }

    //array
    void Reserver(size_t size);
    void Resize(size_t size);

    Value& operator[](int index);
    const Value& operator[](int index) const;

    void ArrayAppend(const Value& value);
    void ArrayAppend(Value&& value);

    void ArrayErase(unsigned int index);

    ArrayValueIter ArrayIterBegin() const { return value_.array_->begin(); }
    ArrayValueIter ArrayIterEnd() const { return value_.array_->end(); }

//重载[]操作符
public:
    bool operator<(const Value& other) const;
    bool operator<=(const Value& other) const;
    bool operator>=(const Value& other) const;
    bool operator>(const Value& other) const;
    bool operator==(const Value& other) const;
    bool operator!=(const Value& other) const;

//输出基本类型Value字符串值
public:
    std::string ToString() const;

public:
    const static Value NullValue;

private:
    void InitPayload(ValueType type);
    void DupPayload(const Value& other);
    void ReleasePayload();

    void set_type(ValueType type);

private:
    ValueType type_;
    union ValueHolder
    {
        int64_t int_;
        uint64_t uint_;
        double real_;
        bool bool_;
        std::string* string_;
        ArrayValue* array_;
        ObjectValue* object_;
    }value_;
};

}//namespace json

}//namespace base
//---------------------------------------------------------------------------
#endif //BASE_JSON_VALUE_H_
