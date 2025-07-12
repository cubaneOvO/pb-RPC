#include "src/rpc/rpc_callback.h"
namespace pbrpc{

    RpcClosure::RpcClosure():cb_(nullptr){

    }

    RpcClosure::RpcClosure(std::function<void()> fun):cb_(fun){

    }
    
    RpcClosure::~RpcClosure(){

    }
    void RpcClosure::Run(){
        if(cb_ != nullptr)cb_();
    }

    void RpcClosure::setCallBack(std::function<void()>fun){
        cb_ = fun;
    }
}