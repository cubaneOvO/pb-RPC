#include "src/rpc/rpc_controller.h"
namespace pbrpc{
    RpcController::RpcController():error_code_(0), is_failed_(false), is_cancle_(false), time_out_(1000),
        local_addr_("0.0.0.0", CConfig::GetInstance()->port_)
    {

    }

    RpcController::~RpcController(){

    }


    void RpcController::Reset(){//复用当前的controller
        error_code_ = 0;
        error_info_ = "";
        msg_id_ = "";
        is_failed_ = false;
        is_cancle_ = false;
        time_out_ = 1000;
        local_addr_ = InterAddress("0.0.0.0", CConfig::GetInstance()->port_);
        peer_addr_ = InterAddress();
    }

    bool RpcController::Failed() const{//判断是否失败
        return is_failed_;
    }

    std::string RpcController::ErrorText() const{//错误信息
        return error_info_;
    }

    void RpcController::StartCancel(){//取消
        is_cancle_ = true;
    }

    void RpcController::SetFailed(const std::string &reason){
        is_failed_ = true;//不确定
        error_info_ = reason;
    }

    bool RpcController::IsCanceled() const{
        return is_cancle_;
    }

    void RpcController::NotifyOnCancel(google::protobuf::Closure *callback){//完成后回调，被取消时会被提前调用

    }

    void RpcController::SetError(int32_t error_code, const std::string error_info){
        error_code_ = error_code;
        SetFailed(error_info);
    }

    int32_t RpcController::GetErrorCode(){
        return error_code_;
    }

    std::string RpcController::GetErrorInfo(){
        return error_info_;
    }

    void RpcController::SetMsgId(const std::string& msg_id){
        msg_id_ = msg_id;
    }

    std::string RpcController::GetMsgId(){
        return msg_id_;
    }

    void RpcController::SetPeerAddr(const std::string ip, int32_t port){
        peer_addr_ = InterAddress(ip, port);
    }

    InterAddress RpcController::GetPeerAddr(){
        return peer_addr_;
    }

    void RpcController::SetLocalAddr(const std::string ip, int32_t port){
        local_addr_ = InterAddress(ip, port);
    }

    InterAddress RpcController::GetLocalAddr(){
        return local_addr_;
    }

    void RpcController::SetTimeOut(int32_t timeout){
        time_out_ = timeout;
    }

    int32_t RpcController::GetTimeOut(){
        return time_out_;
    }

}