//---------------------------------------------------------------------------
#ifndef BASE_FUNCTION_H_
#define BASE_FUNCTION_H_
//---------------------------------------------------------------------------
#include <string>
#include <vector>
#include "memory_block.h"
//---------------------------------------------------------------------------
namespace base
{

//---------------------------------------------------------------------------
//组装字符串，和printf类似
std::string CombineString(const char* format, ...);
//---------------------------------------------------------------------------
//二进制数据转换为字符串(1byte<==>2byte)
std::string BinToString(const unsigned char* buffer, size_t len);
MemoryBlock StringToBin(const std::string& buffer);
MemoryBlock StringToBin(const unsigned char* buffer, size_t len);
//---------------------------------------------------------------------------
//大小写转换
std::string ToUpper(const std::string& str);
std::string ToLower(const std::string& str);
//---------------------------------------------------------------------------
//二进制数据转换为等值的字符(1byte<==>1byte)
std::string BinToChars(const unsigned char* buffer, size_t len);
MemoryBlock CharsToBin(const std::string& buffer);
MemoryBlock CharsToBin(const char* buffer);
//---------------------------------------------------------------------------
std::string Base64_encode(const std::string& dat);
std::string Base64_encode(const char* dat, size_t len);
std::string Base64_encode(const void* dat, size_t len);
std::string Base64_encode(const MemoryBlock& dat);

MemoryBlock Base64_decode(const std::string& dat);
MemoryBlock Base64_decode(const char* dat);
//---------------------------------------------------------------------------
std::vector<std::string> split(const std::string& str, char delim);
//---------------------------------------------------------------------------
//获取程序运行的路径
std::string RunPathFolder();
//获取程序运行的名字
std::string RunExeName();
std::string RunPathFileName(const std::string& name);
std::string RunPathFileName(const char* name);
//---------------------------------------------------------------------------
std::string PathParent(const std::string& path);
std::string PathParent(const char* path);
std::string PathName(const std::string& path);
std::string PathName(const char* path);
//---------------------------------------------------------------------------
//文件夹操作
bool FolderCreate(const std::string& path, bool recursive);
bool FolderDelete(const std::string& path);
bool FolderExist(const std::string& path);
bool FolderExist(const char* path);
//---------------------------------------------------------------------------
//文件操作
bool FileDelete(const std::string& pathname);
bool FileDelete(const char* pathname);
bool FileExist(const std::string& pathname);
bool FileExist(const char* pathname);

bool LoadFile(const std::string& path, MemoryBlock* result);
bool LoadFile(const char* path, MemoryBlock* result);
bool SaveFile(const std::string& path, const char* dat, size_t len);
bool SaveFile(const char* path, const char* dat, size_t len);
//---------------------------------------------------------------------------
//文档（文件和文件夹）
bool DocumentExist(const std::string& pathname);
bool DocumentExist(const char* pathname);
//---------------------------------------------------------------------------
//HTTP时间转换
time_t ParseHTTPDatetime(const char* datetime);
std::string FormatHTTPDatetime(time_t time);
//---------------------------------------------------------------------------

}//namespace base
//---------------------------------------------------------------------------
#endif// BASE_FUNCTION_H_
