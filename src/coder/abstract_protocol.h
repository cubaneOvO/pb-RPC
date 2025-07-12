#pragma once
#include <memory>
#include <string>
namespace pbrpc{
    struct AbstractProtocol : public std::enable_shared_from_this<AbstractProtocol>{
        public:
            using s_ptr = std::shared_ptr<AbstractProtocol>;

            virtual ~AbstractProtocol(){}
        
        public:
            std::string msg_id_; //消息标识
    };
}