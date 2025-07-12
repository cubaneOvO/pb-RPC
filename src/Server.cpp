#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/fcntl.h>
#include <sys/epoll.h>
#include <netinet/tcp.h>
#include <assert.h>
#include <fcntl.h>
#include <signal.h>
//#include "src/echoServer.h"
#include "src/net/TcpServer.h"
#include "src/rpc/rpc_dispatcher.h"
#include "src/common/Config.h"
#include "src/common/log.h"
#include "src/proto/order.pb.h"

pbrpc::TcpServer *echosev;

void Stop(int sig){//终止服务器程序
    printf("sig = %d\n", sig);
    printf("echoServer终止\n");
    echosev->stop();//终止Work/Io线程、事件循环
    delete echosev;
    exit(0);
}




class OrderImpl : public Order{
    public:
    void makeOrder(google::protobuf::RpcController* controller,
        const ::makeOrderRequest* request,
        ::makeOrderResponse* response,
        ::google::protobuf::Closure* done){
            sleep(3);
            if(request->price() < 10){
                response->set_ret_code(-1);
                response->set_res_info("short balance");
                return;
            }
            else{
                response->set_order_id("20250511");
            }
        }

};


int main()
{

    if(pbrpc::CConfig::GetInstance()->init("../../conf/config.yaml") == -1){
        fprintf(stderr, "parser config file failure \n");
        exit(1);
    }
    
    XLOG_INFO("thread {} | ioThread{} | workthread {}", pbrpc::CConfig::GetInstance()->ThreadNum_, pbrpc::CConfig::GetInstance()->ioThread_, pbrpc::CConfig::GetInstance()->workThread_);

    //注册服务
    std::shared_ptr<OrderImpl> service = std::make_shared<OrderImpl>();
    pbrpc::RpcDispatcher::GetInstance()->registerService(service);

    signal(SIGINT, Stop);
    signal(SIGTERM, Stop);
    echosev = new pbrpc::TcpServer();
    echosev->start();//事件循环
    return 0;
}
