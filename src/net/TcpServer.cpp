#include "src/net/TcpServer.h"
namespace pbrpc
{
    TcpServer::TcpServer()
        : mainloop_(new EventLoop()),
          acceptor_(mainloop_.get())
    {
        mainloop_->setepollTimeoutCallback(std::bind(&TcpServer::epollTimeout, this, std::placeholders::_1));

        acceptor_.setNewConnectionCB(std::bind(&TcpServer::newConnection, this, std::placeholders::_1));

        // 设置超时回调
        for (int i = 0, n = CConfig::GetInstance()->ioThread_; i < n; i++)
        {
            EventLoopGroup::GetInstance()->getEventLoop(i)->setepollTimeoutCallback(std::bind(&TcpServer::epollTimeout, this, std::placeholders::_1));
        }

        mainloop_->addTimerEvent(std::make_shared<TimeEvent>(CConfig::GetInstance()->timeout_, true, std::bind(&TcpServer::ClearClientTimerFunc, this)));
    }

    TcpServer::~TcpServer()
    {
    }

    void TcpServer::start()
    {
        mainloop_->run();
    }

    void TcpServer::stop()
    {
        // 停止主事件循环
        mainloop_->stop();
        // 停止从事件循环

        EventLoopGroup::GetInstance()->stopLoop();

        ThreadPool::GetInstance()->stop();
    }

    void TcpServer::newConnection(std::unique_ptr<Socket> clientsock)
    { // 监听sock上有新连接
        // Connection* p = new Connection(mainloop_, clientsock);
        // 将连接后的sock分配给从事件循环去监听
        XLOG_INFO("Getting a new connection, client_ip : {}, client_port : {}", clientsock->getIp(), clientsock->getPort());
        int fd = clientsock->getFd();
        spConnection p(new Connection(EventLoopGroup::GetInstance()->getEventLoop(fd), std::move(clientsock)));
        { // 加锁再往map中添加数据
            std::lock_guard<std::mutex> lock(mutex_);
            conns_[fd] = p; // 添加到server的map容器中
        }
        p->setClosecallback(std::bind(&TcpServer::closeconnection, this, std::placeholders::_1));
        p->seterrorCallback(std::bind(&TcpServer::errorconnection, this, std::placeholders::_1));
        p->setMessageCallback(std::bind(&TcpServer::HandleMessage, this, std::placeholders::_1, std::placeholders::_2));
        p->setsendCallback(std::bind(&TcpServer::sendFinish, this, std::placeholders::_1));

        p->enableReading(); // 设置好回调后监听读事件

        //if (newConnectionCB_) // 创建连接后回调
        //    newConnectionCB_(p);
    }

    void TcpServer::ClearClientTimerFunc()
    {
        time_t now = time(0);
        {
            std::lock_guard<std::mutex> lock(mutex_);
            for (auto it = conns_.begin(); it != conns_.end(); it++)
            {
                if (it->second->timeout(CConfig::GetInstance()->timeout_))
                {
                    XLOG_ERROR("client timeout!");
                    if (!it->second->isWriting())
                    {                                             // 检查数据是否写完
                        ::shutdown(it->second->getFd(), SHUT_WR); // 写完则关闭写侧socket发送TCP FIN 分节
                    }
                }
            }
        }
    }

    void TcpServer::closeconnection(spConnection conn)
    {
        //if (closeconnectionCB_) // 关闭连接前回调
        //    closeconnectionCB_(conn);
        { // 加锁再删除数据
            std::lock_guard<std::mutex> lock(mutex_);
            conns_.erase(conn->getFd());
        }
    }

    void TcpServer::errorconnection(spConnection conn)
    {
        //if (closeconnectionCB_) // 关闭连接前回调
        //    closeconnectionCB_(conn);
        { // 加锁再删除数据
            std::lock_guard<std::mutex> lock(mutex_);
            conns_.erase(conn->getFd());
        }
    }

    void TcpServer::HandleMessage(spConnection conn, std::vector<AbstractProtocol::s_ptr> &request_pkgs)
    {
        if(pbrpc::CConfig::GetInstance()->workThread_ == 0){//处理简单逻辑时，不使用工作线程
            OnMassage(conn, request_pkgs);
        }
        else{
            //把业务添加到工作线程池队列中    
            pbrpc::ThreadPool::GetInstance()->addtask(std::bind(&TcpServer::OnMassage, this, conn, request_pkgs));
        }
    }

    void TcpServer::sendFinish(spConnection conn)
    {
        //if (sendFinishCB_)
        //    sendFinishCB_(conn);
    }

    void TcpServer::epollTimeout(EventLoop *loop)
    {
        //if (epollTimeoutCB_)
        //    epollTimeoutCB_(loop);
    }

    /*
    void TcpServer::setnewConnectionCB(std::function<void(spConnection)> fn)
    {
        newConnectionCB_ = fn;
    }

    void TcpServer::setcloseconnectionCB(std::function<void(spConnection)> fn)
    {
        closeconnectionCB_ = fn;
    }

    void TcpServer::seterrorconnectionCB(std::function<void(spConnection)> fn)
    {
        errorconnectionCB_ = fn;
    }

    void TcpServer::setHandleMessageCB(std::function<void(spConnection, std::string &)> fn)
    {
        HandleMessageCB_ = fn;
    }

    void TcpServer::setsendFinishCB(std::function<void(spConnection)> fn)
    {
        sendFinishCB_ = fn;
    }

    void TcpServer::setepollTimeoutCB(std::function<void(EventLoop *)> fn)
    {
        epollTimeoutCB_ = fn;
    }
    */

    void TcpServer::OnMassage(pbrpc::spConnection conn, std::vector<AbstractProtocol::s_ptr> request_pkgs)
    {
        XLOG_INFO("Work Thread Processing request packets");
        std::vector<AbstractProtocol::s_ptr> respond_pkgs;
        for (int i = 0, n = request_pkgs.size(); i < n; i++)
        {
            XLOG_DEBUG("success get request[{}] form client[{}:{}]", request_pkgs[i]->msg_id_.c_str(), conn->getIp(), conn->getPort());
            std::shared_ptr<TinyPBProtocol> message = std::make_shared<TinyPBProtocol>();
            RpcDispatcher::GetInstance()->dispatch(request_pkgs[i], message, conn);
            respond_pkgs.push_back(message);
        }
        //XLOG_INFO("Work Thread Submit reply packets asynchronously");
        conn->send(respond_pkgs); // 将回复报文加入输出缓冲区中
    }

}
