#pragma once
#include <iostream>
#include <string>
#include <memory>
#include <time.h>
#include <chrono>
#include "spdlog/spdlog.h"
#include "spdlog/async.h"
#include "spdlog/sinks/stdout_color_sinks.h" // or "../stdout_sinks.h" if no color needed
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/rotating_file_sink.h"
#include "src/common/Config.h"
 
namespace pbrpc{
    static inline int NowDateToInt()
    {
        time_t now;
        time(&now);
     
        // choose thread save version in each platform
        tm p;
    #ifdef _WIN32
        localtime_s(&p, &now);
    #else
        localtime_r(&now, &p);
    #endif // _WIN32
        int now_date = (1900 + p.tm_year) * 10000 + (p.tm_mon + 1) * 100 + p.tm_mday;
        return now_date;
    }
     
    static inline int NowTimeToInt()
    {
        time_t now;
        time(&now);
        // choose thread save version in each platform
        tm p;
    #ifdef _WIN32
        localtime_s(&p, &now);
    #else
        localtime_r(&now, &p);
    #endif // _WIN32
     
        int now_int = p.tm_hour * 10000 + p.tm_min * 100 + p.tm_sec;
        return now_int;
    }
     
    class XLogger
    {
    public:
        static XLogger* getInstance()
        {
            static XLogger xlogger;
            return &xlogger;
        }
     
        std::shared_ptr<spdlog::logger> getLogger()
        {
            return m_logger;
        }
     
    private:
        // make constructor private to avoid outside instance
        XLogger()
        {
            // hardcode log path
            const std::string log_out_dir = CConfig::GetInstance()->log_out_dir;// should create the folder if not exist
            const std::string logger_name_prefix = CConfig::GetInstance()->logger_name_prefix;
     
            // decide print to console or log file
            bool console = CConfig::GetInstance()->log_out_console;
     
            // decide the log level
            std::string level = CConfig::GetInstance()->log_level_;
     
            try
            {
                // logger name with timestamp
                int date = NowDateToInt();
                int time = NowTimeToInt();
                const std::string logger_name = logger_name_prefix + "_" + std::to_string(date) + "_" + std::to_string(time);
     
                spdlog::init_thread_pool(8192, 1);
                std::vector<spdlog::sink_ptr> sinks;
                sinks.push_back(std::make_shared<spdlog::sinks::rotating_file_sink_mt>(log_out_dir + "/" + logger_name + ".log", 500 * 1024 * 1024, 1000));

                if (console)
                    sinks.push_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt >());

                m_logger = std::make_shared<spdlog::async_logger>(logger_name, sinks.begin(), sinks.end(), spdlog::thread_pool(), spdlog::async_overflow_policy::block);
                
                // custom format
                m_logger->set_pattern("%Y-%m-%d %H:%M:%S.%f <thread %t> [%l] [%@] %v"); // with timestamp, thread_id, filename and line number
     
                if (level == "trace")
                {
                    m_logger->set_level(spdlog::level::trace);
                    m_logger->flush_on(spdlog::level::trace);
                }
                else if (level == "debug")
                {
                    m_logger->set_level(spdlog::level::debug);
                    m_logger->flush_on(spdlog::level::debug);
                }
                else if (level == "info")
                {
                    m_logger->set_level(spdlog::level::info);
                    m_logger->flush_on(spdlog::level::info);
                }
                else if (level == "warn")
                {
                    m_logger->set_level(spdlog::level::warn);
                    m_logger->flush_on(spdlog::level::warn);
                }
                else if (level == "error")
                {
                    m_logger->set_level(spdlog::level::err);
                    m_logger->flush_on(spdlog::level::err);
                }
            }
            catch (const spdlog::spdlog_ex& ex)
            {
                std::cout << "Log initialization failed: " << ex.what() << std::endl;
            }
        }
     
        ~XLogger()
        {
            spdlog::drop_all(); // must do this
        }
     
        void* operator new(size_t size)
        {}
     
        XLogger(const XLogger&) = delete;
        XLogger& operator=(const XLogger&) = delete;
     
    private:
        std::shared_ptr<spdlog::logger> m_logger;
    };
     
    // use embedded macro to support file and line number
    #define XLOG_TRACE(...) SPDLOG_LOGGER_CALL(pbrpc::XLogger::getInstance()->getLogger().get(), spdlog::level::trace, __VA_ARGS__)
    #define XLOG_DEBUG(...) SPDLOG_LOGGER_CALL(pbrpc::XLogger::getInstance()->getLogger().get(), spdlog::level::debug, __VA_ARGS__)
    #define XLOG_INFO(...) SPDLOG_LOGGER_CALL(pbrpc::XLogger::getInstance()->getLogger().get(), spdlog::level::info, __VA_ARGS__)
    #define XLOG_WARN(...) SPDLOG_LOGGER_CALL(pbrpc::XLogger::getInstance()->getLogger().get(), spdlog::level::warn, __VA_ARGS__)
    #define XLOG_ERROR(...) SPDLOG_LOGGER_CALL(pbrpc::XLogger::getInstance()->getLogger().get(), spdlog::level::err, __VA_ARGS__)
    
}
