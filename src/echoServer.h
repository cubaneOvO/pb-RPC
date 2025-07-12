#pragma once
#include "src/net/TcpServer.h"
#include "src/net/EventLoop.h"
#include "src/net/Connection.h"
#include "src/common/ThreadPool.h"
#include <functional>

class echoServer
{
private:
    pbrpc::TcpServer tcpServer_;

public:
    echoServer();
    ~echoServer();

    void start();
    void stop();

    //void HandleNewConnection(Socket*);//处理新用户连接，在TcpServer类中回调此函数
    //void HandleCloseConnection(spConnection);//关闭用户连接，在TcpServer类中回调此函数
    //void HandleError(spConnection);//处理连接错误，在TcpServer类中回调此函数
    void HandleMessage(pbrpc::spConnection, std::string&);//处理消息，在TcpServer类中回调此函数
    //void HandleSendFinish(spConnection);//消息发送完毕，在TcpServer类中回调此函数
    //void HandleEpollTimeout(EventLoop*);//epoll_wait超时，在TcpServer类中回调此函数

    void OnMassage(pbrpc::spConnection, std::string);//业务处理函数
};
