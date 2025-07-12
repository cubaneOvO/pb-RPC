#include "src/rpc/rpc_channel.h"
namespace pbrpc
{

    RpcChannel::RpcChannel()
        : controller_(nullptr), request_(nullptr), response_(nullptr), closure_(nullptr),
        is_init_(false)
        //client_(nullptr)
    {
    }

    RpcChannel::~RpcChannel(){}


    void RpcChannel::init(controller_s_ptr controller, messagre_s_ptr req, messagre_s_ptr rsp, closure_s_ptr done){
        if(is_init_){
            return;
        }
        controller_ = controller;
        request_ = req;
        response_ = rsp;
        closure_ = done;
        is_init_ = true;
    }

    void RpcChannel::CallMethod(const google::protobuf::MethodDescriptor* method,
        google::protobuf::RpcController* controller, const google::protobuf::Message* request,
        google::protobuf::Message* response, google::protobuf::Closure* done){
            /*std::pair<std::string, int32_t> addr;
            {
                std::string server_method = method->full_name();
                server_method = server_method.substr(0, server_method.find('.'));
                addr = RpcServerList::GetInstance()->getServerAddr(server_method);
                
                if(addr.first.empty() || addr.second == -1){
                    XLOG_ERROR("failed find server: {}", server_method);
                    return;
                }
            }
            InterAddress peer_addr_(addr.first, addr.second);
            client_ = std::make_shared<TcpClient>(peer_addr_);
            */
            std::shared_ptr<pbrpc::TinyPBProtocol> req_protocol = std::make_shared<pbrpc::TinyPBProtocol>();

            RpcController* m_controller = dynamic_cast<RpcController*>(controller);

            if(m_controller == NULL){
                XLOG_ERROR("failed callmethod RpcController convert error");
                return;
            }

           // if(m_controller->GetMsgId().empty()){//没有就随机一个
                req_protocol->msg_id_ = MsgIdUtil::GetInstance()->GetMsgId();
                m_controller->SetMsgId(req_protocol->msg_id_);
            //}
            //else{
            //    req_protocol->msg_id_ = m_controller->GetMsgId();
            //}

            req_protocol->method_name_ = method->full_name();
            XLOG_DEBUG("{} | call method name : {}", req_protocol->msg_id_, req_protocol->method_name_);

            std::string server_method = method->full_name();
            server_method = server_method.substr(0, server_method.find('.'));
            spConnection peer_conn = TcpClient::GetInstance()->getConnection(server_method);//获取连接
            if(!peer_conn){
                m_controller->SetError(ERROR_FAILED_CONNECT, ErrorCode::GetInstance()->getErrorInfo(ERROR_FAILED_CONNECT));
                XLOG_ERROR("{} | connct error, error coder : {}, error info : {}", req_protocol->msg_id_, m_controller->GetErrorCode(), m_controller->GetErrorInfo());
                return;
            }

            if(!is_init_){
                std::string error_info("RpcChannel not init");
                m_controller->SetError(ERROR_RPC_CHANNEL_INIT, error_info);
                XLOG_DEBUG("{} | {}, RpcChannel not init", req_protocol->msg_id_, error_info);
                return;
            }

            //序列化到pb_data_
            if(!request->SerializeToString(&(req_protocol->pb_data_))){
                std::string error_info("failed to serialize");
                m_controller->SetError(ERROR_FAILED_SERIALIZE, error_info);
                XLOG_DEBUG("{} | {}, origin request : {}", req_protocol->msg_id_, error_info, request->ShortDebugString());
                return;
            }

            s_ptr channel(shared_from_this());

            timer_event_ = std::make_shared<TimeEvent>(m_controller->GetTimeOut(), false, [peer_conn, channel, m_controller]() mutable{
                m_controller->StartCancel();
                m_controller->SetError(ERROR_RPC_CALL_TIMEOUT, "rpc call timeout " + std::to_string(m_controller->GetTimeOut()));
                if(channel->closure_){
                    channel->closure_->Run();
                }
                channel.reset();
            });
            EventLoopGroup::GetInstance()->getEventLoop(peer_conn->getFd())->addTimerEvent(timer_event_);

            
            peer_conn->pushSendMessage(req_protocol, [peer_conn, channel, m_controller](AbstractProtocol::s_ptr req_msg) mutable {
                    std::shared_ptr<pbrpc::TinyPBProtocol> req_protocol = std::dynamic_pointer_cast<pbrpc::TinyPBProtocol>(req_msg);
                    XLOG_DEBUG("{} | send rpc request success. call method name : {}", req_protocol->msg_id_, req_protocol->method_name_);
                    
                    peer_conn->pushReadMessage(req_protocol->msg_id_, [peer_conn, channel, m_controller](AbstractProtocol::s_ptr rsp_msg) mutable {
                        channel->timer_event_->setCancled(true);//收到回包取消定时任务

                        std::shared_ptr<pbrpc::TinyPBProtocol> rsp_message = std::dynamic_pointer_cast<pbrpc::TinyPBProtocol>(rsp_msg);
                        XLOG_INFO("{} | get rpc response success. call method name : {}", rsp_message->msg_id_, rsp_message->method_name_);
                        
                        if(!channel->response_->ParseFromString(rsp_message->pb_data_)){
                            XLOG_ERROR("deserialize error");
                            m_controller->SetError(ERROR_FAILED_SERIALIZE, "deserialize error");

                            return;
                          }

                          if(rsp_message->error_code_ != 0){
                            XLOG_ERROR("{} | call rpc method : {} failed, error code : {}, error info{}", rsp_message->msg_id_, rsp_message->error_code_, ErrorCode::GetInstance()->getErrorInfo(rsp_message->error_code_));
                            m_controller->SetError(rsp_message->error_code_, ErrorCode::GetInstance()->getErrorInfo(rsp_message->error_code_));
                          }

                          if(!channel->controller_->IsCanceled() && channel->closure_)
                            channel->closure_->Run();

                    });
                    peer_conn->enableReading();
                });
            
                peer_conn->enableWriting();
            
        }
}
