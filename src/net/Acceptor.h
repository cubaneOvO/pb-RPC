#pragma once
#include <functional>
#include "src/net/Socket.h"
#include "src/net/InterAddress.h"
#include "src/net/Channel.h"
#include "src/net/EventLoop.h"
#include "src/common/Config.h"
#include <memory>

namespace pbrpc
{
    class Acceptor
    { // 用于监听连接，封装了channel
    private:
        EventLoop *loop_; // accept对应的事件循环，在构造函数中传入,没有所有权
        // 这里使用栈内存创建sock和channel是因为一个网络程序只会有一个acceptor
        Socket listenSock_;     // 服务端用于监听的socket，在构造函数中创建
        Channel acceptChannel_; // acceptor对应的channel，在构造函数中创建
        std::function<void(std::unique_ptr<Socket>)> newConnectionCB_;

    public:
        Acceptor(EventLoop *);
        ~Acceptor();
        void newConnection();                                                  // 处理客户端连接请求
        void setNewConnectionCB(std::function<void(std::unique_ptr<Socket>)>); // 设置新连接请求所需的回调函数
    };
}
