#pragma once
#include "src/coder/abstract_protocol.h"
namespace pbrpc{
    struct TinyPBProtocol : public AbstractProtocol{
        public:
            static const char PB_START = 0x02; 
            int32_t pk_len_{0};//整包长度
            int8_t msg_type_{0};//包类型：1表示心跳包，0表示消息包
            int32_t msg_id_len_{0};//msgID长度
            //msg_id在基类中定义;
            int32_t method_name_len_{0};
            std::string method_name_;
            int32_t error_code_{0};
            //int32_t error_info_len_{0};
            //std::string error_info_;
            std::string pb_data_;
            int32_t check_sum_{0};
            static const char PB_END = 0x03;

            bool parse_success {false};

        public:
            ~TinyPBProtocol() {}
    };
}