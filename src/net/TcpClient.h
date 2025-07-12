#pragma once

#include "src/net/Socket.h"
#include "src/net/InterAddress.h"
#include "src/net/EventLoop.h"
#include "src/coder/abstract_coder.h"
#include "src/net/Channel.h"
#include "src/net/EventLoopGroup.h"
#include "src/common/error_code.h"
#include "src/common/TimeEvent.h"
#include "src/common/RpcServerList.h"
#include "src/common/msg_id_util.h"
#include <functional>
#include <string.h>
#include <memory>
#include <unordered_map>
#include <chrono>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <errno.h>
#include <mutex>

namespace pbrpc
{
    class TcpClient
    {
    public:
        TcpClient(const TcpClient &) = delete;
        TcpClient &operator=(const TcpClient &) = delete;

        virtual ~TcpClient() {

        };

        static TcpClient *GetInstance()
        {
            static TcpClient s_instance;
            return &s_instance;
        }

        spConnection getConnection(std::string server_name);

        void stopLoop();

    private:
        TcpClient();

        std::unordered_map<std::string, spConnection> server_conns_;
        std::unique_ptr<EventLoop> loop_;//心跳包循环
        std::unordered_map<std::string, int> keepalive_count_;
        std::mutex mtx_;
    };
}