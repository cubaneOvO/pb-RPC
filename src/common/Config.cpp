#include "src/common/Config.h"

namespace pbrpc{
    CConfig::CConfig()
    {}
    
    int CConfig::init(std::string configFilePath)
    {
        return load(configFilePath);
    }
    
    int CConfig::load(std::string configFilePath)
    {
        int nRet = -1;
        YAML::Node config;
        try{
             config = YAML::LoadFile(configFilePath);
        } 
        catch(YAML::BadFile &e) {
            std::cerr<<"read error!"<<std::endl;
            return nRet;
        }

        try
        {
            ip_ = config["ip"].as<std::string>();
            port_ = config["port"].as<int>();
            ioThread_ = config["threadnum"]["iothread"].as<int>();
            workThread_ = config["threadnum"]["workthread"].as<int>();
            ThreadNum_ = ioThread_ + workThread_;
            timeout_ = config["timeout"].as<int>();
        
            log_level_ = config["logging"]["level"].as<std::string>();
            log_out_console = config["logging"]["log_out_console"].as<bool>();
            log_out_dir = config["logging"]["log_out_dir"].as<std::string>();
            logger_name_prefix = config["logging"]["logger_name_prefix"].as<std::string>();
    
            etcd_url_ = config["etcd"]["url"].as<std::string>();
            etcd_base_dir_ = config["etcd"]["base_dir"].as<std::string>();
            etcd_lease_keep_time_ = config["etcd"]["leaseKeepTime"].as<int>();

            nRet = 0;
        }
        catch (std::exception &e)
        {
            std::cerr<<"parser config file failure: " << e.what()<<std::endl;
        }
    
        return nRet;
    }
}

