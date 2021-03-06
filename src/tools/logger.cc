//---------------------------------------------------------------------------
#include "logger.h"
#include <cstring>
//---------------------------------------------------------------------------
namespace lsm
{

namespace tools
{
//---------------------------------------------------------------------------
Logger g_logger;
//---------------------------------------------------------------------------
__thread char t_error_buf[512];
//---------------------------------------------------------------------------
Logger::Logger()
{
    logger_ = base::Logger::stdout_logger_mt();
}
//---------------------------------------------------------------------------
const char* Logger::OsError(int e)
{
    return strerror_r(e, t_error_buf, sizeof(t_error_buf));
}
//---------------------------------------------------------------------------

}//namespace tools 

}//namespace lsm
//---------------------------------------------------------------------------
