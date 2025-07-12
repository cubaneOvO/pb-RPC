#pragma once

#include <string>
#include "yaml-cpp/yaml.h"
#include <iostream>

namespace pbrpc{
    class CConfig
    {
    public:
        CConfig(const CConfig &) = delete;
        CConfig &operator=(const CConfig &) = delete;
        virtual ~CConfig(){};
    
        static CConfig *GetInstance()
        {
            static CConfig s_instance;
            return &s_instance;
        }
    
        int init(std::string);

    private:
        int load(std::string);
    
    public:
    //通用
        std::string ip_;
        int port_;
        int ThreadNum_;
        int ioThread_;
        int workThread_;
        int timeout_;
    
    //logging
        std::string log_level_;
        bool log_out_console;
        std::string log_out_dir;
        std::string logger_name_prefix;

    //etcd
        std::string etcd_url_;
        std::string etcd_base_dir_;
        int etcd_lease_keep_time_{-1};
    private:
        CConfig();
    };
}
