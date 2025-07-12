#pragma once
#include "src/net/EventLoop.h"
#include "src/net/Socket.h"
#include "src/net/Channel.h"
#include "src/net/Acceptor.h"
#include "src/net/Connection.h"
#include "src/common/ThreadPool.h"
#include "src/net/EventLoopGroup.h"
#include "src/common/TimeEvent.h"
#include "src/common/log.h"
#include <unordered_map>
#include "functional"
#include <vector>
#include <memory>
#include <mutex>
namespace pbrpc
{
    class TcpServer
    {
    private:
        std::unique_ptr<EventLoop> mainloop_;              // 主事件循环
        Acceptor acceptor_;                                // 一个server只能有一个acceprot对象
        std::unordered_map<int, spConnection> conns_;
        std::mutex mutex_; // 给map加锁


        /*
        std::function<void(spConnection)> newConnectionCB_;
        std::function<void(spConnection)> closeconnectionCB_;
        std::function<void(spConnection)> errorconnectionCB_;
        std::function<void(spConnection, std::string &)> HandleMessageCB_;
        std::function<void(spConnection)> sendFinishCB_;
        std::function<void(EventLoop *)> epollTimeoutCB_;
        */

        void ClearClientTimerFunc();
        void OnMassage(pbrpc::spConnection conn, std::vector<AbstractProtocol::s_ptr> request_pkgs);
    public:
        TcpServer();
        ~TcpServer();
        void start();
        void stop(); // 停止IO线程和事件循环

        void newConnection(std::unique_ptr<Socket>);     // 处理新用户连接，在accept类中回调此函数
        void closeconnection(spConnection);              // 关闭用户连接，在connection类中回调此函数
        void errorconnection(spConnection);              // 处理连接错误，在connection类中回调此函数
        void HandleMessage(spConnection, std::vector<AbstractProtocol::s_ptr>&); // 处理消息，在connection类中回调此函数
        void sendFinish(spConnection);                   // 消息发送完毕，在connection类中回调此函数
        void epollTimeout(EventLoop *);                  // epoll_wait超时，在EventLoop类中回调此函数

        /*
        // 设置回调函数, 回调业务层的函数
        void setnewConnectionCB(std::function<void(spConnection)>);
        void setcloseconnectionCB(std::function<void(spConnection)>);
        void seterrorconnectionCB(std::function<void(spConnection)>);
        void setHandleMessageCB(std::function<void(spConnection, std::string &)>);
        void setsendFinishCB(std::function<void(spConnection)>);
        void setepollTimeoutCB(std::function<void(EventLoop *)>);
        */
    };

}
