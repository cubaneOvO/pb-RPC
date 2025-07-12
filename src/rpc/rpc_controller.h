#pragma once
#include "google/protobuf/service.h"
#include "google/protobuf/stubs/callback.h"

#include "src/common/Config.h"
#include "src/net/InterAddress.h"
namespace pbrpc
{
    class RpcController : public google::protobuf::RpcController
    {
    public:
        RpcController();
        ~RpcController();

        void Reset();

        bool Failed() const;

        std::string ErrorText() const;

        void StartCancel();

        void SetFailed(const std::string &reason);

        bool IsCanceled() const;

        void NotifyOnCancel(google::protobuf::Closure *callback);

        void SetError(int32_t error_code, const std::string error_info);

        int32_t GetErrorCode();

        std::string GetErrorInfo();

        void SetMsgId(const std::string& msg_id);

        std::string GetMsgId();

        void SetPeerAddr(const std::string ip, int32_t port);

        InterAddress GetPeerAddr();

        void SetLocalAddr(const std::string ip, int32_t port);

        InterAddress GetLocalAddr();

        void SetTimeOut(int32_t timeout);

        int32_t GetTimeOut();
    private:
        int error_code_;
        std::string error_info_;
        std::string msg_id_;
        bool is_failed_; // 调用是否失败
        bool is_cancle_; // 调用是否取消

        InterAddress local_addr_;
        InterAddress peer_addr_;

        int time_out_;
    };
}