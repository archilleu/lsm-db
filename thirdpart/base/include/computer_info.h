//---------------------------------------------------------------------------
#ifndef BASE_COMPUTER_INFO_H_
#define BASE_COMPUTER_INFO_H_
//---------------------------------------------------------------------------
#include <string>
#include <vector>
//---------------------------------------------------------------------------
namespace base
{

class ComputerInfo
{
public:
    ComputerInfo();
    ~ComputerInfo();

    //Computer description
    struct ComputerName
    {
        std::string sysname;    /* Name of the implementation of the operating system.  */
        std::string netname;    /* Name of this node on the network.  */
        std::string release;    /* Current release level of this implementation.  */
        std::string version;    /* Current version level of this release.  */
        std::string machine;    /* Name of the hardware type the system is running on.  */
    };
    static ComputerName GetComputerName();

    //harddisk
    struct DiskspaceInfo
    {
        std::string filesystem;
        std::string mount_point;
        size_t total;
        size_t used;
    };
    static std::vector<DiskspaceInfo> GetDiskspaceInfo();

    //memory(kb)
    struct MemoryInfo
    {
        size_t mem_total;
        size_t mem_free;
        size_t swap_total;
        size_t swap_free;
    };
    static MemoryInfo GetMemoryInfo();

    //cup info
    struct CPUInfo
    {
        std::string vender;             //cup vender
        std::string modle_name;         //cpu name
        float       MHz;                //frequency
        int         sockets;            //the number of physical socket
        int         core_per_socket;    //the core number of per cpu
        int         thread_per_core;    //the thread number of per core
    };
    static CPUInfo GetCPUInfo();

    //CPU current usage
    static void     InitCPUUsage();
    static float    GetCPUUsage();

private:
    static void GetCPUValue(unsigned long long value[4]);

private:
    //more info man /proc
    static unsigned long long last_[4];
};

}//namespace base
//---------------------------------------------------------------------------
#endif //BASE_COMPUTER_INFO_H_
