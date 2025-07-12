#include "src/common/RpcServerList.h"

namespace pbrpc
{
    RpcServerList::RpcServerList()
    {
        discovery_ = std::make_unique<RpcDiscovery>(std::bind(&RpcServerList::getServerNodeCallBack, this, std::placeholders::_1, std::placeholders::_2),
                                                    std::bind(&RpcServerList::delServerNodeCallBack, this, std::placeholders::_1, std::placeholders::_2));
    }
    std::pair<std::string, int32_t> RpcServerList::getServerAddr(std::string method_name)
    {
        
        if (server_list.find(method_name) == server_list.end())
        {
            XLOG_ERROR("no server: {}", method_name);
            return std::make_pair("", -1);
        }
        auto node = server_list.find(method_name)->second->getLowestCPUNode();
        return std::make_pair(node->ip, node->port);
    }

    void RpcServerList::getServerNodeCallBack(std::string key, std::string value)
    {
        std::string method;
        double cpu_usage;
        std::string ip;
        int32_t port;

        parseukv(key, value, cpu_usage, ip, port, method);

        if (server_list.find(method) == server_list.end())
        {
            server_list[method] = std::make_unique<ServerNodeManager>();
            server_list[method]->upsertNode({cpu_usage, ip, port});
        }
        else server_list[method]->upsertNode({cpu_usage, ip, port});
    }

    void RpcServerList::delServerNodeCallBack(std::string key, std::string value)
    {
        std::string method;
        double cpu_usage;
        std::string ip;
        int32_t port;

        parseukv(key, value, cpu_usage, ip, port, method);

        if(server_list.find(method) == server_list.end()){
            XLOG_ERROR("{}:{} | not find method: {}", ip, port, method);
            return;
        }
        if(server_list[method]->removeNode(ip, port)){
            XLOG_INFO("{}:{} | server {} down", ip, port, method);
        }
        else{
            XLOG_ERROR("{}:{} | not find method: {}", ip, port, method);
        }

    }

    void RpcServerList::parseukv(const std::string& key, const std::string& value, double& cpu_usage, std::string& ip, int32_t& port, std::string& method){
       
        XLOG_DEBUG("get server info key: {}, value: {}", key, value);
        std::vector<std::string> parts;
        std::stringstream s1(key);
        std::string part;

        // 按斜杠分割字符串
        while (std::getline(s1, part, '/'))
        {
            if (!part.empty())
            {
                parts.push_back(part);
            }
        }
        // 检查是否有足够的部分
        if (parts.size() >= 2)
        {
            method = parts[1];
            XLOG_DEBUG("get server: {}", method);
            parts.clear();
        }
        else
        {
            XLOG_ERROR("get method str faild");
            return;
        }

        std::stringstream s2(value);
        parts.clear();

        while (std::getline(s2, part, '_'))
        {
            if (!part.empty())
            {
                parts.push_back(part);
            }
        }
        // 检查是否有足够的部分
        if (parts.size() == 2)
        {
            ip = parts[0];
            cpu_usage = std::stod(parts[1]);
            XLOG_DEBUG("get cpu_usage: {}", cpu_usage);
            parts.clear();
        }
        else
        {
            XLOG_ERROR("get cpu_usage faild");
            return;
        }

        std::stringstream s3(ip);
        parts.clear();

        while (std::getline(s3, part, ':'))
        {
            if (!part.empty())
            {
                parts.push_back(part);
            }
        }
        // 检查是否有足够的部分
        if (parts.size() == 2)
        {
            ip = parts[0];
            port = std::stoi(parts[1]);
            XLOG_DEBUG("get ip:{}, port:{}", ip, port);
            parts.clear();
        }
        else
        {
            XLOG_ERROR("get ip:port faild");
            return;
        }
    }
}