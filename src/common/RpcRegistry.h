#pragma once
#include "etcd/Client.hpp"
#include "etcd/KeepAlive.hpp"
#include "etcd/Response.hpp"
#include "src/common/Config.h"
#include "src/common/log.h"
#include <iostream>
#include <fstream>
#include <unistd.h>
namespace pbrpc
{
    class RpcRegistry
    {
    public:
        RpcRegistry(const RpcRegistry &) = delete;
        RpcRegistry &operator=(const RpcRegistry &) = delete;

        static RpcRegistry *GetInstance()
        {
            static RpcRegistry s_instance;
            return &s_instance;
        }

        virtual ~RpcRegistry()
        {
            _keep_alive->Cancel();
        }

        bool registry(const std::string &key, const std::string &val)
        {
            auto response = _client->put(key, val, _lease_id).get();
            if (response.is_ok() == false)
            {
                XLOG_DEBUG("Server:{} Registry faild", key);
                return false;
            }
            return true;
        }

        double getCpuUsage(){
            CpuUsage();
            sleep(3);//采样时间
            return CpuUsage();
        }


    private:
        RpcRegistry() : _client(std::make_shared<etcd::Client>(CConfig::GetInstance()->etcd_url_)),
                        _keep_alive(_client->leasekeepalive(CConfig::GetInstance()->etcd_lease_keep_time_).get()),
                        _lease_id(_keep_alive->Lease())
        {
        }
        std::shared_ptr<etcd::Client> _client;
        std::shared_ptr<etcd::KeepAlive> _keep_alive;
        uint64_t _lease_id;

        double CpuUsage() {
            unsigned long long upTime, sysUptime, userTime, sysTime, idleTime;
            std::ifstream stat("/proc/stat");
            stat.ignore(5); // Skip "cpu" identifier
            stat >> userTime >> sysTime >> idleTime;
            upTime = userTime + sysTime + idleTime;
            stat.close();
            double cpuUsage = 0.0;
            static unsigned long long lastUpdate = 0, lastTotal = 0;
            unsigned long long currentTime = upTime, currentTotal = userTime + sysTime;
            if (lastUpdate != 0) {
              unsigned long long timeDelta = currentTime - lastUpdate;
              unsigned long long totalDelta = currentTotal - lastTotal;
              cpuUsage = (double) totalDelta / timeDelta;
            }
            lastUpdate = currentTime;
            lastTotal = currentTotal;
            return cpuUsage;
        }
    };
}