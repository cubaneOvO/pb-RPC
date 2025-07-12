#include "src/net/Connection.h"

namespace pbrpc
{
    Connection::Connection(EventLoop *loop, std::unique_ptr<Socket> cilentSock, TcpConnectionType type)
        : loop_(loop), cilentSock_(std::move(cilentSock)), disConnect(false), connectChannel_(new Channel(loop_, cilentSock_->getFd())), type_(type)
    {
        connectChannel_->setReadcallback(std::bind(&Connection::onMessage, this));
        connectChannel_->setClosecallback(std::bind(&Connection::closeCallback, this));
        connectChannel_->setErrorcallback(std::bind(&Connection::errorCallback, this));
        connectChannel_->setWritecallback(std::bind(&Connection::sendMessage, this));
        connectChannel_->useET(); // 设置边缘触发

        coder_ = new TinyPBCoder();
    }

    void Connection::enableReading()
    {
        connectChannel_->enableReading(); // 监听连接sock上的读事件
    }

    void Connection::enableWriting()
    {
        connectChannel_->enableWriting();
    }

    void Connection::disableReading()
    {
        connectChannel_->disableReading();
    }
    void Connection::disableWriting()
    {
        connectChannel_->disableWriting();
    }

    Connection::~Connection()
    {
        connectChannel_->remove();
        if (coder_)
        {
            delete coder_;
            coder_ = nullptr;
        }
    }

    int Connection::getFd() const
    {
        return cilentSock_->getFd();
    }

    std::string Connection::getIp() const
    {
        return cilentSock_->getIp();
    }

    uint16_t Connection::getPort() const
    {
        return cilentSock_->getPort();
    }

    void Connection::closeCallback()
    { // tcp连接关闭时的回调函数
        disConnect = true;
        connectChannel_->remove(); // 将channel从epoll红黑树上删除
        if (closeCallback_)
            closeCallback_(shared_from_this());
    }
    void Connection::errorCallback()
    { // tcp连接出错时的回调函数
        disConnect = true;
        connectChannel_->remove(); // 将channel从epoll红黑树上删除
        if (errorCallback_)
            errorCallback_(shared_from_this()); // 关闭客户端的fd。
    }

    void Connection::setClosecallback(std::function<void(spConnection)> fn)
    {
        closeCallback_ = fn;
    }

    void Connection::seterrorCallback(std::function<void(spConnection)> fn)
    {
        errorCallback_ = fn;
    }

    void Connection::setMessageCallback(std::function<void(spConnection, std::vector<AbstractProtocol::s_ptr> &)> fn)
    {
        messageCallback_ = fn;
    }

    void Connection::setsendCallback(std::function<void(spConnection)> fn)
    {
        sendCallback_ = fn;
    }

    void Connection::onMessage()
    { // 连接sock上有新数据
        while (1)
        {
            int errorTag = 0;
            ssize_t nread = inputbuffer_.readFd(getFd(), &errorTag);
            if (nread == 0)
            { // 客户端关闭了链接
                disConnect = true;
                closeCallback();
                break;
            }
            else if (nread < 0)
            {
                if (errorTag == EINTR) // 被信号中断
                    continue;
                else if (errorTag == EAGAIN || errorTag == EWOULDBLOCK)
                { // buffer中数据已经读完
                    timeval val;
                    gettimeofday(&val, NULL);
                    lastTime_ = val.tv_sec * 1000 + val.tv_usec / 1000;
                    // TODO：协议解析并回调
                    parseMsg();
                    break;
                }
            }
        }
    }

    void Connection::parseMsg()
    {
        if (type_ == ConnectionByServer)
        {
            // 回显
            /*std::string message;
            message = inputbuffer_.retrieveAsString(inputbuffer_.readableBytes());
            regSend(message, message.size());
            */

            /*auto getMessage = [&message, this]() -> bool
            {
                if (inputbuffer_.readableBytes() < sizeof(int32_t))
                {
                    return false;
                }
                int32_t size = inputbuffer_.peekInt32();
                if (inputbuffer_.readableBytes() < sizeof(int32_t) + size)
                {
                    return false;
                }
                inputbuffer_.retrieve(sizeof(int32_t));
                message = inputbuffer_.retrieveAsString(size);
                return true;
            };

            while (true)
            {
                if (!getMessage())
                {
                    break;
                }
                messageCallback_(shared_from_this(), message);
            }*/

            /*std::vector<AbstractProtocol::s_ptr> result;
            std::vector<AbstractProtocol::s_ptr> replay_messages;
            coder_->decode(result, inputbuffer_);

            for(int i = 0; i < result.size(); i++){
                XLOG_INFO("success get request[{}] form client[{}:{}]", result[i]->msg_id_.c_str(), getIp(), getPort());
                std::shared_ptr<TinyPBProtocol> message = std::make_shared<TinyPBProtocol>();
                RpcDispatcher::GetInstance()->dispatch(result[i], message, shared_from_this());
                //message->pb_data_ = "hello this is pbrpc test data";
                //message->msg_id_ = result[i]->msg_id_;
                replay_messages.push_back(message);
            }
            coder_->encode(replay_messages, outputbuffer_);
            enableWriting();
            */
            std::vector<AbstractProtocol::s_ptr> request_pkgs;
            coder_->decode(request_pkgs, inputbuffer_);
            messageCallback_(shared_from_this(), request_pkgs);
        }
        else
        {
            
            // 从 buffer 里 decode 得到 message 对象, 执行其回调
            std::vector<AbstractProtocol::s_ptr> result;
            coder_->decode(result, inputbuffer_);

            for (size_t i = 0; i < result.size(); ++i)
            {
                std::string msg_id = result[i]->msg_id_;
                {
                    std::unique_lock<std::shared_mutex> lock_(read_dones_mtx_);
                    auto it = read_dones_.find(msg_id);
                    if (it != read_dones_.end())
                    {
                        it->second(result[i]);
                        read_dones_.erase(it);
                    }
                }
            }
        }
    }

    void Connection::send(std::vector<AbstractProtocol::s_ptr> &respond_pkgs)
    {
        if (disConnect)
            return;
        loop_->runInLoop(std::bind(&Connection::regSend, this, respond_pkgs));
    }

    // 注：若regSend与sendMessage不在同一线程中则有可能造成竞争
    void Connection::regSend(std::vector<AbstractProtocol::s_ptr> respond_pkgs)
    {
        if (disConnect)
            return;
        // outputbuffer_.appendWithsep(message.data(), size);
        // outputbuffer_.appendInt32(size);
        // outputbuffer_.writeToBuffer(message.c_str(), size);
        coder_->encode(respond_pkgs, outputbuffer_);
        enableWriting(); // 向epoll注册写事件
    }

    void Connection::sendMessage()
    {
       // XLOG_INFO("Write event trigger");
        if (disConnect)
            return;

        if (type_ == ConnectionByClient)
        {
            std::vector<AbstractProtocol::s_ptr> messages;
            {
                std::unique_lock<std::shared_mutex> lock_(write_dones_mtx_);
                for (size_t i = 0; i < write_dones_.size(); ++i)
                {
                    XLOG_INFO("send request packets, msg_id:{}", write_dones_[i].first->msg_id_);
                    messages.push_back(write_dones_[i].first);
                    write_dones_[i].second(write_dones_[i].first);
                }
                write_dones_.clear();
            }
            coder_->encode(messages, outputbuffer_);
        }

        while (true)
        {
            int errorTag = 0;
            int writen = outputbuffer_.writeFd(getFd(), &errorTag);
            if (outputbuffer_.readableBytes() == 0)
            {                     // 发送缓冲区中没数据了，数据传输完毕
                disableWriting(); // 不再监听写事件
                if (sendCallback_)
                    sendCallback_(shared_from_this()); // 通知写完
                break;
            }
            if (writen < 0 && errorTag == EAGAIN)
            { // 缓冲区写满了暂时没法继续写
                break;
            }
        }
    }

    bool Connection::timeout(int64_t timeval_)
    {
        timeval val;
        gettimeofday(&val, NULL);

        int64_t now = val.tv_sec * 1000 + val.tv_usec / 1000;

        if (now - lastTime_ > timeval_)
        {
            return true;
        }
        else
        {
            return false;
        }
    }

    bool Connection::isWriting() const
    {
        if (connectChannel_)
            return connectChannel_->isWriting();
        else
            return false;
    }

    void Connection::setType(TcpConnectionType type)
    {
        type_ = type;
    }
    TcpConnectionType Connection::getType()
    {
        return type_;
    }

    void Connection::pushSendMessage(AbstractProtocol::s_ptr message, std::function<void(AbstractProtocol::s_ptr)> done)
    {
        std::unique_lock<std::shared_mutex> lock_(write_dones_mtx_);
        write_dones_.push_back(std::make_pair(message, done));
    }

    void Connection::pushReadMessage(const std::string &msg_id, std::function<void(AbstractProtocol::s_ptr)> done)
    {
        std::unique_lock<std::shared_mutex> lock_(read_dones_mtx_);
        read_dones_.insert(std::make_pair(msg_id, done));
    }

    void Connection::popReadMessage(const std::string &msg_id)
    {
        std::unique_lock<std::shared_mutex> lock_(read_dones_mtx_);
        auto it = read_dones_.find(msg_id);
        if (it != read_dones_.end())
        {
            read_dones_.erase(it);
        }
        return;
    }
}
