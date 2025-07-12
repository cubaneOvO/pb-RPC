#pragma once
#include "src/coder/abstract_protocol.h"
#include "src/coder/tinypb_protocol.h"
#include "src/common/log.h"
#include "src/common/error_code.h"
#include "src/rpc/rpc_controller.h"
#include "src/net/Connection.h"
#include "src/common/RpcRegistry.h"

#include "google/protobuf/service.h"
#include "google/protobuf/descriptor.h"
#include "google/protobuf/message.h"

#include <unordered_map>
namespace pbrpc
{
    class Connection;
    typedef std::shared_ptr<Connection> spConnection;

    class RpcDispatcher{
        public:

            RpcDispatcher(const RpcDispatcher &) = delete;
            RpcDispatcher &operator=(const RpcDispatcher &) = delete;
            virtual ~RpcDispatcher(){};
    
            static RpcDispatcher *GetInstance()
            {
                static RpcDispatcher s_instance;
                return &s_instance;
            }

            using service_s_ptr = std::shared_ptr<google::protobuf::Service>;

            void dispatch(AbstractProtocol::s_ptr request, AbstractProtocol::s_ptr respond, spConnection conn);

            void registerService(service_s_ptr service);

            
        private:

            RpcDispatcher(){}

            //void setTinyPBError(std::shared_ptr<TinyPBProtocol>msg, int32_t error_code, const std::string error_info);
            bool parseServiceFullName(const std::string& full_name, std::string& service_name, std::string& method_name);

            std::unordered_map<std::string, service_s_ptr> service_map_;//服务列表

    };
}
