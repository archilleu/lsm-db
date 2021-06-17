//---------------------------------------------------------------------------
#ifndef BASE_MD5_H_
#define BASE_MD5_H_
//---------------------------------------------------------------------------
#include <cstdint>
#include <string>
//---------------------------------------------------------------------------
namespace base
{

class MD5
{
public:
    using MD5_VAL = uint8_t[16];

    MD5();
    ~MD5();

    //计算MD5时,循环调用BufferAppend直到需要计算的内容为空,然后再调用Done即可得到md5值
    void BufferAppend(const uint8_t* buffer, size_t len); 
    void BufferAppend(const char* buffer, size_t len);
    void Done(MD5_VAL md5_val);
    std::string Done();

    //string
    void StringMD5(const std::string& dat, MD5_VAL md5_val);
    void StringMD5(const char* dat, MD5_VAL md5_val);
    std::string StringMD5(const char* dat);
    std::string StringMD5(const std::string& dat);

    //dat
    void DataMD5(const char* dat, size_t len, MD5_VAL md5_val);
    std::string DataMD5(const char* dat, size_t len);

    //计算文件MD5
    void FileMD5(const char* path, MD5_VAL md5_val);
    void FileMD5(const std::string& path, MD5_VAL md5_val);
    std::string FileMD5(const char* path);
    std::string FileMD5(const std::string& path);

private:
    void Transform(uint32_t state[4], const uint8_t block[64]);
    void Reset();

private:
    struct MD5_CTX
    {
        uint64_t count;
        uint32_t state[4];
        uint8_t buffer[64];
    }context_;
};

}
//---------------------------------------------------------------------------
#endif //BASE_MD5_H_
