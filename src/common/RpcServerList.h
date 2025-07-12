#pragma once
#include <unordered_map>
#include <queue>
#include <vector>
#include <memory>
#include <string>
#include "src/common/RpcDiscovery.h"
#include "src/common/ServerNode.h"
namespace pbrpc{
    class RpcServerList{
        public:
        RpcServerList(const RpcServerList &) = delete;
        RpcServerList &operator=(const RpcServerList &) = delete;
        virtual ~RpcServerList() {};

        static RpcServerList *GetInstance()
        {
            static RpcServerList s_instance;
            return &s_instance;
        }

        std::pair<std::string, int32_t> getServerAddr(std::string method_name);
    private:
        RpcServerList();

        void getServerNodeCallBack(std::string key, std::string value);

        void delServerNodeCallBack(std::string key, std::string value);

        void parseukv(const std::string& key, const std::string& value, double& cpu_usage, std::string& ip, int32_t& port, std::string& method);

        //key: method_name; value: 小根堆<cpu usage, <ip,port>>
        std::unordered_map<std::string, std::unique_ptr<ServerNodeManager>> server_list;

        std::unique_ptr<RpcDiscovery> discovery_;
    };
}