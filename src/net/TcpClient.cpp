#include "src/net/TcpClient.h"

namespace pbrpc
{
    /* TcpClient::TcpClient(InterAddress addr) : peer_addr_(addr), sock_(new Socket(createNonblokingSock())),
         event_loop_(EventLoopGroup::GetInstance()->getEventLoop(sock_->getFd())), connectChannel_(event_loop_, sock_->getFd()),
         isConnected(false)
     {
         sock_->setIpAndPort(peer_addr_.getIp(), peer_addr_.getPort());
         sock_->setReuseaddr(true);

     }
     TcpClient::~TcpClient(){
         //EventLoopGroup::GetInstance()->stopLoop();
         if(connectChannel_.getInepoll()){
             connectChannel_.remove();
         }
     }

     void TcpClient::connect(std::function<void()> done){
         int rt = ::connect(sock_->getFd(), peer_addr_.getAddr(), sizeof(sockaddr_in));
         if(rt == 0){
             XLOG_DEBUG("connect {}:{} success", peer_addr_.getIp(), peer_addr_.getPort());
             isConnected = true;
             if(done) done();
         }
         else if(rt == -1 && errno == EINPROGRESS){
             XLOG_DEBUG("connect rt == -1, erron = EINPROGRESS");
             connectChannel_.setWritecallback([this, done](){
                 int error = 0;
                 socklen_t erroe_len = sizeof(error);
                 getsockopt(sock_->getFd(), SOL_SOCKET, SO_ERROR, &error, &erroe_len);
                 if(error == 0){
                     XLOG_DEBUG("connect {}:{} success", peer_addr_.getIp(), peer_addr_.getPort());
                     isConnected = true;
                 }
                 else{
                     XLOG_ERROR("connect error, errno = {}, error = {}", errno, strerror(errno));
                     connect_errorcode_ = ERROR_FAILED_CONNECT;
                     connect_error_info_ = "connect error, sys error = " + std::string(strerror(errno));
                 }
                 connectChannel_.remove();//直接移除
                 if(done) done();
             });
             connectChannel_.setErrorcallback([this, done](){
                 connectChannel_.remove();
                 connect_errorcode_ = ERROR_FAILED_CONNECT;
                 if(errno == ECONNREFUSED){
                     connect_error_info_ = "connect refused, sys error = " + std::string(strerror(errno));
                 }
                 else{
                     connect_error_info_ = "connect unkonwn, sys error = " + std::string(strerror(errno));
                 }
                 XLOG_ERROR("connect error, errno = {}, error = {}", errno, strerror(errno));
                 if(done) done();
             });
             connectChannel_.enableWriting();
         }
         else{
             XLOG_ERROR("connect error, errno = {}, error = {}", errno, strerror(errno));
             connect_errorcode_ = ERROR_FAILED_CONNECT;
             connect_error_info_ = "connect error, sys error = " + std::string(strerror(errno));
             if(done) done();
         }

     }

     void TcpClient::writeMessage(AbstractProtocol::s_ptr msg, std::function<void(AbstractProtocol::s_ptr)> done){
         if(isConnected){
             if(!conn_){
                 int fd = sock_->getFd();
                 conn_ = std::make_shared<Connection>(event_loop_, std::move(sock_),pbrpc::ConnectionByClient);
             }

             conn_->pushSendMessage(msg, done);
             conn_->enableWriting();
             XLOG_INFO("write message");
         }
         else{
             XLOG_ERROR("connect failed can't call writeMessage");
         }
     }

     void TcpClient::readMessage(const std::string& msg_id, std::function<void(AbstractProtocol::s_ptr)> done){
         if(isConnected){
             if(!conn_){
                 int fd = sock_->getFd();
                 conn_ = std::make_shared<Connection>(event_loop_, std::move(sock_), pbrpc::ConnectionByClient);
             }
             conn_->pushReadMessage(msg_id, done);
             conn_->enableReading();
         }
         else{
             XLOG_ERROR("connect failed can't call readMessage");
         }
     }

     int TcpClient::getConnectErrorCode(){
         return connect_errorcode_;
     }
     std::string TcpClient::getConnectErrorInfo(){
         return connect_error_info_;
     }

     void TcpClient::addTimeEvent(TimeEvent::s_ptr timeEvent){
         event_loop_->addTimerEvent(timeEvent);
     }
         */

    TcpClient::TcpClient() : loop_(new EventLoop())
    {
       // XLOG_INFO("Registering timed task");
        TimeEvent::s_ptr timer_event_ = std::make_shared<TimeEvent>(CConfig::GetInstance()->timeout_, true, [this]() {
            std::shared_ptr<pbrpc::TinyPBProtocol> keepalive_pkg = std::make_shared<pbrpc::TinyPBProtocol>();
            keepalive_pkg->msg_type_ = (int8_t)1;
            keepalive_pkg->msg_id_ =  MsgIdUtil::GetInstance()->GetMsgId();
            for(auto& kv: server_conns_){
                kv.second->pushSendMessage(keepalive_pkg, [&kv, this](AbstractProtocol::s_ptr request_pkg){
                    std::shared_ptr<pbrpc::TinyPBProtocol> m_request = std::dynamic_pointer_cast<pbrpc::TinyPBProtocol>(request_pkg);
                    kv.second->pushReadMessage(m_request->msg_id_,  [&kv, this](AbstractProtocol::s_ptr respond_pkg){
                        {
                            std::unique_lock<std::mutex> lock_(mtx_);
                            keepalive_count_[kv.first] = -1;
                        }
                    });
                    XLOG_DEBUG("send keepalive pkg success");
                });
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(CConfig::GetInstance()->timeout_/2));
            for(auto it = server_conns_.begin(); it != server_conns_.end();){
                it->second->popReadMessage(keepalive_pkg->msg_id_);
                {
                    std::unique_lock<std::mutex> lock_(mtx_);
                    keepalive_count_[it->first]++;
                    if(keepalive_count_[it->first] >= 3){
                        keepalive_count_[it->first] = 0;
                        server_conns_.erase(it++);
                    }   
                    else it++;
                }
            }
        });

        loop_->addTimerEvent(timer_event_);
        ThreadPool::GetInstance()->addtask(std::bind(&EventLoop::run, loop_.get()));
    }
    //(EventLoop *loop, std::unique_ptr<Socket> cilentSock, TcpConnectionType type)
    spConnection TcpClient::getConnection(std::string server_name)
    {
       // XLOG_INFO("Get a service:{} keepalive connection", server_name);
        {
            std::unique_lock<std::mutex> lock_(mtx_);
            if ((server_conns_.find(server_name) != server_conns_.end()) && (keepalive_count_[server_name] < 3))
            {
                return server_conns_[server_name];
            }
        }

        std::pair<std::string, int32_t> addr = RpcServerList::GetInstance()->getServerAddr(server_name);
        InterAddress peer_addr_(addr.first, addr.second);
        std::unique_ptr<Socket> socket_(new Socket(createNonblokingSock()));
        int rt = ::connect(socket_->getFd(), peer_addr_.getAddr(), sizeof(sockaddr_in));

        if (rt == 0)
        {
            XLOG_DEBUG("connect {}:{} success", peer_addr_.getIp(), peer_addr_.getPort());
            spConnection conn = std::make_shared<Connection>(EventLoopGroup::GetInstance()->getEventLoop(socket_->getFd()), std::move(socket_), ConnectionByClient);
            {
                std::unique_lock<std::mutex> lock_(mtx_);
                server_conns_[server_name] = conn;
                keepalive_count_[server_name] = 0;
            }
            return conn;

        }
        else if (rt == -1 && errno == EINPROGRESS)
        {
            fd_set fdw;
            FD_ZERO(&fdw);
            FD_SET(socket_->getFd(),&fdw);//检测fd上写事件
            struct timeval timeout;
            timeout.tv_sec = CConfig::GetInstance()->timeout_ / 1000;
            timeout.tv_usec = (CConfig::GetInstance()->timeout_ % 1000) * 1000;
            int rc = select(socket_->getFd() + 1, NULL, &fdw, NULL, &timeout);
            if(rc < 0){
                XLOG_ERROR("connect connect {}:{} error", peer_addr_.getIp(), peer_addr_.getPort());
                XLOG_ERROR("select error");
                return nullptr;
            }
            if(rc == 0){//超时
                XLOG_ERROR("connect {}:{} timeout", peer_addr_.getIp(), peer_addr_.getPort());
                return nullptr;
            }
            else if(rc == 1 && FD_ISSET(socket_->getFd(), &fdw)){
                XLOG_DEBUG("connect {}:{} success", peer_addr_.getIp(), peer_addr_.getPort());
                spConnection conn = std::make_shared<Connection>(EventLoopGroup::GetInstance()->getEventLoop(socket_->getFd()), std::move(socket_), ConnectionByClient);
                {
                    std::unique_lock<std::mutex> lock_(mtx_);
                    server_conns_[server_name] = conn;
                    keepalive_count_[server_name] = 0;
                }
                return conn;
            }
            else{
                int error = 0;
                socklen_t erroe_len = sizeof(error);
                getsockopt(socket_->getFd(), SOL_SOCKET, SO_ERROR, &error, &erroe_len);
                XLOG_ERROR("connect connect {}:{} error, errno = {}, error = {}", peer_addr_.getIp(), peer_addr_.getPort(), errno, strerror(errno));
                return nullptr;
            }
        }
        else {
            XLOG_ERROR("connect connect {}:{} error, errno = {}, error = {}", peer_addr_.getIp(), peer_addr_.getPort(), errno, strerror(errno));
            return nullptr;
        }
    }

    void TcpClient::stopLoop(){
        loop_->stop();
    }

}