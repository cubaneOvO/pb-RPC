#pragma once
#include <functional>
#include "src/net/Socket.h"
#include "src/net/InterAddress.h"
#include "src/net/Channel.h"
#include "src/common/Buffer.h"
#include "src/net/EventLoop.h"
#include <memory>
#include <atomic>
#include <vector>
#include <sys/time.h>
#include "src/common/log.h"
#include "src/coder/abstract_protocol.h"
#include "src/coder/tinypb_coder.h"
#include "src/rpc/rpc_dispatcher.h"
#include <shared_mutex>
namespace pbrpc
{

    enum TcpConnectionType
    {
        ConnectionByServer = 1, // 作为服务端使用
        ConnectionByClient = 2, // 作为客户端使用
    };

    class RpcDispatcher;
    class EventLoop;
    class Channel;
    class Connection; // 前置声明
    typedef std::shared_ptr<Connection> spConnection;

    class Connection : public std::enable_shared_from_this<Connection>
    {
        // public:
        // using s_ptr = std::shared_ptr<Connection>;
    private:
        EventLoop *loop_;                         // connect对应的事件循环，在构造函数中传入
        std::unique_ptr<Socket> cilentSock_;      // 服务端用于通信的socket，在构造函数中传入(并由它来管理socket的生命周期)
        std::unique_ptr<Channel> connectChannel_; // connection对应的channel，在构造函数中创建
        Buffer inputbuffer_;                      // 接收缓冲区
        Buffer outputbuffer_;                     // 发生缓冲区
        std::atomic_bool disConnect;              // 如果为true则标志tcp连接已经断开

        std::function<void(spConnection)> closeCallback_;
        std::function<void(spConnection)> errorCallback_;
        std::function<void(spConnection, std::vector<AbstractProtocol::s_ptr>&)> messageCallback_;
        std::function<void(spConnection)> sendCallback_;

        int64_t lastTime_;//时间戳

        TcpConnectionType type_; // 默认服务端


        AbstractCoder *coder_;

        // std::pair<AbstractProtocol::s_ptr, std::function<void(AbstractProtocol::s_ptr)>>
        std::vector<std::pair<AbstractProtocol::s_ptr, std::function<void(AbstractProtocol::s_ptr)>>> write_dones_;
        std::shared_mutex write_dones_mtx_;
        // key 为 msg_id
        std::map<std::string, std::function<void(AbstractProtocol::s_ptr)>> read_dones_;
        std::shared_mutex read_dones_mtx_;

    public:
        Connection(EventLoop *loop, std::unique_ptr<Socket> clientsock, TcpConnectionType type = ConnectionByServer); // 可以传入coder进行解析
        ~Connection();
        int getFd() const;
        std::string getIp() const;
        uint16_t getPort() const;
        void send(std::vector<AbstractProtocol::s_ptr>&);  // 发送消息接口
        void regSend(std::vector<AbstractProtocol::s_ptr>); // 向epoll注册写事件
        bool timeout(int64_t);         // 判断tcp连接是否超时
        bool isWriting() const;            // 判断是否还有数据要写

        void closeCallback(); // tcp连接关闭时的回调函数
        void errorCallback(); // tcp连接出错时的回调函数
        void onMessage();     // 接收数据的回调函数
        void sendMessage();   // 发送数据的回调函数

        void setClosecallback(std::function<void(spConnection)>);
        void seterrorCallback(std::function<void(spConnection)>);
        void setMessageCallback(std::function<void(spConnection, std::vector<AbstractProtocol::s_ptr>&)>);
        void setsendCallback(std::function<void(spConnection)>);

        void enableReading(); // 注册读事件
        void enableWriting(); // 注册写事件

        void disableReading(); // 取消读事件
        void disableWriting(); // 取消写事件

        void parseMsg();

        void setType(TcpConnectionType);
        TcpConnectionType getType();

        void pushSendMessage(AbstractProtocol::s_ptr, std::function<void(AbstractProtocol::s_ptr)>);
        void pushReadMessage(const std::string &, std::function<void(AbstractProtocol::s_ptr)>);

        void popReadMessage(const std::string &);
    };

}
