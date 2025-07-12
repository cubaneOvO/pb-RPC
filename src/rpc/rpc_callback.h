#pragma once
#include "google/protobuf/stubs/callback.h"

#include <functional>
namespace pbrpc
{
    class RpcClosure : public google::protobuf::Closure
    {
    public:
        RpcClosure();
        RpcClosure(std::function<void()>);
        ~RpcClosure();
        void Run();
        void setCallBack(std::function<void()>);

    private:
        std::function<void()> cb_;
    };
}
