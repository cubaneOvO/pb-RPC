#include "src/rpc/rpc_dispatcher.h"
namespace pbrpc
{

    void RpcDispatcher::dispatch(AbstractProtocol::s_ptr request, AbstractProtocol::s_ptr respond, spConnection conn){
        std::shared_ptr<TinyPBProtocol> req_protocol = std::dynamic_pointer_cast<TinyPBProtocol>(request);
        std::shared_ptr<TinyPBProtocol> rsp_protocol = std::dynamic_pointer_cast<TinyPBProtocol>(respond);

        rsp_protocol->msg_type_ = req_protocol->msg_type_;
        rsp_protocol->msg_id_ = req_protocol->msg_id_;
        rsp_protocol->method_name_ = req_protocol->method_name_;

        if(rsp_protocol->msg_type_ == 1){
          return;
        }
        
        std::string method_full_name = req_protocol->method_name_;
        std::string service_name;
        std::string method_name;

        if (!parseServiceFullName(method_full_name, service_name, method_name)) {
          rsp_protocol->error_code_ = ERROR_PARSE_SERVICE_NAME;
          //setTinyPBError(rsp_protocol, ERROR_PARSE_SERVICE_NAME, "parse service name error");
          return;
        }
      
        auto it = service_map_.find(service_name);
        if (it == service_map_.end()) {
          XLOG_ERROR("{} | sericve neame:{} not found", req_protocol->msg_id_.c_str(), service_name.c_str());
          rsp_protocol->error_code_ = ERROR_SERVICE_NOT_FOUND;
          //setTinyPBError(rsp_protocol, ERROR_SERVICE_NOT_FOUND, "service not found");
          return;
        }
      
        service_s_ptr service = (*it).second;
      
        const google::protobuf::MethodDescriptor* method = service->GetDescriptor()->FindMethodByName(method_name);
        
        if (method == NULL) {
          XLOG_ERROR("{} | method neame:{} not found in service:{}", req_protocol->msg_id_.c_str(), method_name.c_str(), service_name.c_str());
          rsp_protocol->error_code_ = ERROR_SERVICE_NOT_FOUND;
          //setTinyPBError(rsp_protocol, ERROR_SERVICE_NOT_FOUND, "method not found");
          return;
        }
      
        google::protobuf::Message* req_msg = service->GetRequestPrototype(method).New();

        //反序列化将pb_data反序列化request对象

        if(!req_msg->ParseFromString(req_protocol->pb_data_)){
          XLOG_ERROR("{} | deserilize error", req_protocol->msg_id_.c_str(), method_name.c_str(), service_name.c_str());
          rsp_protocol->error_code_ = ERROR_FAILED_DESERIALIZE;
          //setTinyPBError(rsp_protocol, ERROR_FAILED_DESERIALIZE, "deserilize error");
          if(req_msg){
            delete req_msg;
            req_msg = nullptr;
          }
          return;
        }

        XLOG_DEBUG("{} | get rpc request:[{}]", req_protocol->msg_id_.c_str(), req_msg->ShortDebugString().c_str());

        google::protobuf::Message* rsp_msg = service->GetResponsePrototype(method).New();

        RpcController rpccontroller;
        rpccontroller.SetPeerAddr(conn->getIp(), conn->getPort());
        rpccontroller.SetMsgId(req_protocol->msg_id_);


        service->CallMethod(method, &rpccontroller, req_msg, rsp_msg,  NULL);
        
        

        if(!rsp_msg->SerializeToString(&(rsp_protocol->pb_data_))){
          XLOG_ERROR("{} | serilize error, origin message {}", req_protocol->msg_id_.c_str(), rsp_msg->ShortDebugString().c_str());
          rsp_protocol->error_code_ = ERROR_FAILED_SERIALIZE;
          //setTinyPBError(rsp_protocol, ERROR_FAILED_SERIALIZE, "serilize error");
        }else {
          rsp_protocol->error_code_ = 0;
          XLOG_DEBUG("{} | dispatch success, requesut:[{}], response:[{}]", req_protocol->msg_id_.c_str(), req_msg->ShortDebugString().c_str(), rsp_msg->ShortDebugString().c_str());
        }
        if(req_msg){
          delete req_msg;
          req_msg = nullptr;
        }
        if(rsp_msg){
          delete rsp_msg;
          rsp_msg = nullptr;
        }

    }

    void RpcDispatcher::registerService(service_s_ptr service){
        
        std::string service_name = service->GetDescriptor()->full_name();
        service_map_[service_name] = service;

        std::string key = CConfig::GetInstance()->etcd_base_dir_ + '/' + service_name + '/' + CConfig::GetInstance()->ip_ + '-' + std::to_string(CConfig::GetInstance()->port_);
        std::string value = CConfig::GetInstance()->ip_ + ':' + std::to_string(CConfig::GetInstance()->port_) + '_' + std::to_string(RpcRegistry::GetInstance()->getCpuUsage());
        if(RpcRegistry::GetInstance()->registry(key, value)){
            XLOG_INFO("success Regist server -> key:{}, val:{}", key, value);
        }
        else{
            XLOG_ERROR("Regist server -> key:{}, val:{} failed", key, value);
        }
    }

    bool RpcDispatcher::parseServiceFullName(const std::string& full_name, std::string& service_name, std::string& method_name) {
      if (full_name.empty()) {
        XLOG_ERROR("full name empty"); 
        return false;
      }
      size_t i = full_name.find_first_of(".");
      if (i == full_name.npos) {
        XLOG_ERROR("not find . in full name {}", full_name.c_str());
        return false;
      }
      service_name = full_name.substr(0, i);
      method_name = full_name.substr(i + 1, full_name.length() - i - 1);
    
      XLOG_DEBUG("parse request sericve_name:{} and method_name:{} from full name {}", service_name.c_str(), method_name.c_str(),full_name.c_str());
    
      return true;
    
    }

    //void RpcDispatcher::setTinyPBError(std::shared_ptr<TinyPBProtocol>msg, int32_t error_code, const std::string error_info){
    //    msg->error_code_ = error_code;
    //}

}


