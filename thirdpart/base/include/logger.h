//---------------------------------------------------------------------------
#ifndef BASE_LOGGER_H_
#define BASE_LOGGER_H_
//---------------------------------------------------------------------------
#include <string>
#include <vector>
#include <memory>
#include <cstdarg>
//---------------------------------------------------------------------------
#ifdef _DEBUG
#define LOGGER_STR_H(x) #x
#define LOGGER_STR_HELPER(x) LOGGER_STR_H(x)
#define LOGGER_TRACE(logger, ...) logger->trace("[" __FILE__ ":" LOGGER_STR_HELPER(__LINE__) "]: " __VA_ARGS__)
#define LOGGER_DEBUG(logger, ...) logger->debug(__VA_ARGS__)
#else
#define LOGGER_TRACE(logger, ...)
#define LOGGER_DEBUG(logger, ...)
#endif
//---------------------------------------------------------------------------
namespace base
{

//---------------------------------------------------------------------------
class Logger
{
public:
    enum Level
    {
        TRACE =0,
        DEBUG,
        INFO,
        WARN,
        ERROR,
        CRITICAL,
        OFF
    };

    using SinkPtr = std::shared_ptr<class Sink>;
    Logger(std::vector<SinkPtr> slots)
    :   level_(TRACE),
        flush_level_(ERROR),
        slots_(slots)
    {
    }
    Logger(SinkPtr slot)
    :   Logger(std::vector<SinkPtr>(1, slot))
    {
    }
    virtual ~Logger(){}
    Logger(Logger&) =delete;
    Logger& operator=(Logger&) =delete;

    void set_level(Level lv) { level_ = lv; }
    Level level() const { return level_; }

    void set_flush_level(Level lv) { flush_level_ = lv; }
    Level flush_level() const { return flush_level_; }

    void Flush();

    //log
    inline void log        (Level lv, const char* format, ...);
    inline void trace      (const char* format, ...);
    inline void debug      (const char* format, ...);
    inline void info       (const char* format, ...);
    inline void warn       (const char* format, ...);
    inline void error      (const char* format, ...);
    inline void critical   (const char* format, ...);
    inline void off        (const char* format, ...);

public:
    //Create logger
    //console
    static std::shared_ptr<Logger> stdout_logger_mt();
    static std::shared_ptr<Logger> stdout_logger_st();

    //file
    static std::shared_ptr<Logger> file_logger_mt(const std::string& path, bool daily=true);
    static std::shared_ptr<Logger> file_logger_st(const std::string& path, bool daily=true);

    //file & console
    static std::shared_ptr<Logger> file_stdout_logger_mt(const std::string& path, bool daily=true);
    static std::shared_ptr<Logger> file_stdout_logger_st(const std::string& path, bool daily=true);

private:
    bool ShouldLog(Level lv) const { return level_ <= lv; }
    bool ShouldFlush(Level lv) const { return flush_level_ <= lv; }

    void WriteToSinks(const char* msg, Level lv);

private:
    Level level_;
    Level flush_level_;
    std::vector<SinkPtr> slots_;
};
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
#define BUILD_LOG(_X_) va_list ap;                  \
    va_start(ap, format);                           \
        int err_code = vasprintf(&_X_, format, ap); \
    va_end(ap);                                     \
    if(0 > err_code)                                \
        return;                                     \
//---------------------------------------------------------------------------
inline void Logger::log(Level lv, const char* format, ...)
{
    if(!ShouldLog(lv))
        return;

    //build log
    char* buffer = 0;
    BUILD_LOG(buffer);

    WriteToSinks(buffer, lv);
    free(buffer);

    return;
}
//---------------------------------------------------------------------------
inline void Logger::trace(const char* format, ...)
{
    if(!ShouldLog(TRACE))
        return;

    //build log
    char* buffer = 0;
    BUILD_LOG(buffer);

    WriteToSinks(buffer, TRACE);
    free(buffer);

    return;
}
//---------------------------------------------------------------------------
inline void Logger::debug(const char* format, ...)
{
    if(!ShouldLog(DEBUG))
        return;

    //build log
    char* buffer = 0;
    BUILD_LOG(buffer);

    WriteToSinks(buffer, DEBUG);
    free(buffer);

    return;
}
//---------------------------------------------------------------------------
inline void Logger::info(const char* format, ...)
{
    if(!ShouldLog(INFO))
        return;

    //build log
    char* buffer = 0;
    BUILD_LOG(buffer);

    WriteToSinks(buffer, INFO);
    free(buffer);

    return;
}
//---------------------------------------------------------------------------
inline void Logger::warn(const char* format, ...)
{
    if(!ShouldLog(WARN))
        return;

    //build log
    char* buffer = 0;
    BUILD_LOG(buffer);

    WriteToSinks(buffer, WARN);
    free(buffer);

    return;
}
//---------------------------------------------------------------------------
inline void Logger::error(const char* format, ...)
{
    if(!ShouldLog(ERROR))
        return;

    //build log
    char* buffer = 0;
    BUILD_LOG(buffer);

    WriteToSinks(buffer, ERROR);
    free(buffer);

    return;
}
//---------------------------------------------------------------------------
inline void Logger::critical(const char* format, ...)
{
    if(!ShouldLog(CRITICAL))
        return;

    //build log
    char* buffer = 0;
    BUILD_LOG(buffer);

    WriteToSinks(buffer, CRITICAL);
    free(buffer);

    return;
}
//---------------------------------------------------------------------------
void Logger::off(const char* format, ...)
{
    if(!ShouldLog(OFF))
        return;

    //build log
    char* buffer = 0;
    BUILD_LOG(buffer);

    WriteToSinks(buffer, OFF);
    free(buffer);

    return;
}

}//namespace base
//---------------------------------------------------------------------------
#endif //BASE_LOGGER_H_
