// 网络通讯的客户端程序。
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <time.h>
#include "src/net/TcpClient.h"
#include "src/common/Config.h"
#include "src/common/log.h"
#include "src/proto/order.pb.h"
#include "src/rpc/rpc_channel.h"
#include "src/rpc/rpc_callback.h"
#include "functional"

void makeOrder(int price, std::string goods, int timeout_, std::function<void(std::shared_ptr<makeOrderResponse>)> fn){
    NEWRPCCHANNEL(channel);
    NEWMESSAGE(makeOrderRequest, request);
    NEWMESSAGE(makeOrderResponse, response);
    request->set_price(price);
    request->set_goods(goods);

    NEWRPCCONTROLLER(controller);
    controller->SetTimeOut(timeout_);

    std::shared_ptr<pbrpc::RpcClosure> closure = std::make_shared<pbrpc::RpcClosure>([fn, request, response, channel, controller]() mutable{
        if(controller->GetErrorCode() == 0){
            XLOG_INFO("call rpc success, request[{}], response[{}]", request->ShortDebugString(), response->ShortDebugString()); 
            fn(response);
        }
        else{
            XLOG_ERROR("call rpc falied, error info[{}], error code[{}]", controller->GetErrorInfo(), controller->GetErrorCode()); 
        }
       channel.reset();//减少引用计数
    });
    
    CALLRPRC(channel, Order_Stub, makeOrder, controller, request, response, closure);
}

    

int main(int argc, char *argv[])
{
    if (pbrpc::CConfig::GetInstance()->init("../../conf/config.yaml") == -1)
    {
        fprintf(stderr, "parser config file failure \n");
        exit(1);
    }
    

    makeOrder(100, "apple", 500000, [](std::shared_ptr<makeOrderResponse> response){
        XLOG_INFO("success apple");
    });
    makeOrder(9, "orange", 500000, [](std::shared_ptr<makeOrderResponse> response){
        XLOG_INFO("success orange");
    });
    
    sleep(30);
    pbrpc::TcpClient::GetInstance()->stopLoop();
    pbrpc::EventLoopGroup::GetInstance()->stopLoop();
    pbrpc::ThreadPool::GetInstance()->stop();
    return 0;
}
