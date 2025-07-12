#pragma once
#include <unordered_map>
#include <string>
namespace pbrpc
{
#ifndef SYS_ERROR_PREFIX
#define SYS_ERROR_PREFIX(xx) 1000##xx
#endif

    const int ERROR_PEER_CLOSED = SYS_ERROR_PREFIX(0000);        // 连接时对端关闭
    const int ERROR_FAILED_CONNECT = SYS_ERROR_PREFIX(0001);     // 连接失败
    const int ERROR_FAILED_GET_REPLY = SYS_ERROR_PREFIX(0002);   // 获取回包失败
    const int ERROR_FAILED_DESERIALIZE = SYS_ERROR_PREFIX(0003); // 反序列化失败
    const int ERROR_FAILED_SERIALIZE = SYS_ERROR_PREFIX(0004);   // 序列化 failed

    const int ERROR_FAILED_ENCODE = SYS_ERROR_PREFIX(0005); // encode failed
    const int ERROR_FAILED_DECODE = SYS_ERROR_PREFIX(0006); // decode failed

    const int ERROR_RPC_CALL_TIMEOUT = SYS_ERROR_PREFIX(0007); // rpc 调用超时

    const int ERROR_SERVICE_NOT_FOUND = SYS_ERROR_PREFIX(0008);  // service 不存在
    const int ERROR_METHOD_NOT_FOUND = SYS_ERROR_PREFIX(0009);   // method 不存在 method
    const int ERROR_PARSE_SERVICE_NAME = SYS_ERROR_PREFIX(0010); // service name 解析失败
    const int ERROR_RPC_CHANNEL_INIT = SYS_ERROR_PREFIX(0011);   // rpc channel 初始化失败
    const int ERROR_RPC_PEER_ADDR = SYS_ERROR_PREFIX(0012);      // rpc 调用时候对端地址异常

    class ErrorCode
    {
    public:
        ErrorCode(const ErrorCode &) = delete;
        ErrorCode &operator=(const ErrorCode &) = delete;
        virtual ~ErrorCode() {}

        static ErrorCode *GetInstance()
        {
            static ErrorCode s_instance;
            return &s_instance;
        }

        std::string getErrorInfo(int errorCode){
            if(errorInfo_.find(errorCode) == errorInfo_.end()){
                return {"unknow error"};
            }
            return errorInfo_[errorCode];
        }

    private:
        ErrorCode() {
            errorInfo_[ERROR_PEER_CLOSED] = "peer closed";
            errorInfo_[ERROR_FAILED_CONNECT] = "connect error";
            errorInfo_[ERROR_FAILED_GET_REPLY] = "get reply error";
            errorInfo_[ERROR_FAILED_DESERIALIZE] = "deserilize error";
            errorInfo_[ERROR_FAILED_SERIALIZE] = "serilize error";

            errorInfo_[ERROR_FAILED_ENCODE] = "encode error";
            errorInfo_[ERROR_FAILED_DECODE] = "decode error";

            errorInfo_[ERROR_RPC_CALL_TIMEOUT] = "rpc call timeout";

            errorInfo_[ERROR_SERVICE_NOT_FOUND] = "service not found";
            errorInfo_[ERROR_METHOD_NOT_FOUND] = "method not found";
            errorInfo_[ERROR_PARSE_SERVICE_NAME] = "parse service name error";
            errorInfo_[ERROR_RPC_CHANNEL_INIT] = "rpcChannel init error";
            errorInfo_[ERROR_RPC_PEER_ADDR] = "peer addr error";
        }
        std::unordered_map<int, std::string> errorInfo_;
    };
}
