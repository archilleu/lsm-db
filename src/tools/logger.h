//---------------------------------------------------------------------------
#ifndef LSM_TOOLS_LOGGER_H_
#define LSM_TOOLS_LOGGER_H_
//---------------------------------------------------------------------------
#include "base/include/logger.h"
#include "base/include/thread.h"
//---------------------------------------------------------------------------
#define Logger_trace(format, ...) tools::g_logger.get_logger()->trace("[%s:%s:%d:%s]:" format " ", base::CurrentThread::tid_str(),__FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__)
#define Logger_debug(format, ...) tools::g_logger.get_logger()->debug("[%s:%s:%d]:" format " ", base::CurrentThread::tid_str(),__FILE__, __LINE__, ##__VA_ARGS__)
#define Logger_info(...) tools::g_logger.get_logger()->info(__VA_ARGS__)
#define Logger_warn(format, ...) tools::g_logger.get_logger()->warn("[%s:%s:%d]:" format " ", base::CurrentThread::tid_str(), __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define Logger_error(format, ...) tools::g_logger.get_logger()->error("[%s:%s:%d]:" format " ", base::CurrentThread::tid_str(), __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define Logger_critical(format, ...) tools::g_logger.get_logger()->critical("[%s:%d]:" format " ", base::CurrentThread::tid_str(), __LINE__, ##__VA_ARGS__)
#define Logger_off(format, ...) tools::g_logger.get_logger()->off("[%s:%d]:" format " ", base::CurrentThread::tid_str(), __LINE__, ##__VA_ARGS__)
//---------------------------------------------------------------------------

namespace lsm
{

namespace tools 
{

//---------------------------------------------------------------------------
class Logger
{
public:
    Logger();

public:
    void set_logger(std::shared_ptr<base::Logger> logger) { logger_ = logger; }
    std::shared_ptr<base::Logger> get_logger() { return logger_; }

    const char* OsError(int e) const;

private:
    std::shared_ptr<base::Logger> logger_;
};

//---------------------------------------------------------------------------
extern Logger g_logger;
//---------------------------------------------------------------------------

}//namespace tools

}//namespace lsm 
//---------------------------------------------------------------------------
#endif //LSM_TOOLS_LOGGER_H_

