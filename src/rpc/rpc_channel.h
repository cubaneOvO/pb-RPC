#pragma once
#include "google/protobuf/service.h"
#include "google/protobuf/message.h"
#include "google/protobuf/descriptor.h"

#include "src/net/InterAddress.h"
#include "src/common/msg_id_util.h"
#include "src/coder/tinypb_protocol.h"
#include "src/rpc/rpc_controller.h"
#include "src/net/TcpClient.h"
#include "src/common/error_code.h"
#include "src/common/TimeEvent.h"
#include "src/common/RpcServerList.h"
#include <memory>

namespace pbrpc
{

#define NEWMESSAGE(type, var_name) \
    std::shared_ptr<type> var_name = std::make_shared<type>();

#define NEWRPCCONTROLLER(var_name) \
    std::shared_ptr<pbrpc::RpcController> var_name = std::make_shared<pbrpc::RpcController>();

#define NEWRPCCHANNEL(var_name) \
    std::shared_ptr<pbrpc::RpcChannel> var_name = std::make_shared<pbrpc::RpcChannel>();

#define CALLRPRC(channel, stub_name, method_name, controller, request, response, closure)                        \
    {                                                                                                         \
        channel->init(controller, request, response, closure);                                                \
        stub_name(channel.get()).method_name(controller.get(), request.get(), response.get(), closure.get()); \
    }

    class RpcChannel : public google::protobuf::RpcChannel, public std::enable_shared_from_this<RpcChannel>
    {
    public:
        using s_ptr = std::shared_ptr<RpcChannel>;
        using controller_s_ptr = std::shared_ptr<google::protobuf::RpcController>;
        using messagre_s_ptr = std::shared_ptr<google::protobuf::Message>;
        using closure_s_ptr = std::shared_ptr<google::protobuf::Closure>;

        RpcChannel();

        ~RpcChannel();

        void CallMethod(const google::protobuf::MethodDescriptor *method,
                        google::protobuf::RpcController *controller, const google::protobuf::Message *request,
                        google::protobuf::Message *response, google::protobuf::Closure *done);

        void init(controller_s_ptr controller, messagre_s_ptr req, messagre_s_ptr rsp, closure_s_ptr done); // CallMetho前先调用init

    private:
        InterAddress peer_addr_;

        controller_s_ptr controller_;
        messagre_s_ptr request_;
        messagre_s_ptr response_;
        closure_s_ptr closure_;
        //TcpClient::s_ptr client_;
        TimeEvent::s_ptr timer_event_;

        bool is_init_;
    };

}