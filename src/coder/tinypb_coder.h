#pragma once
#include "src/coder/abstract_coder.h"
#include "src/coder/tinypb_protocol.h"
#include "src/common/log.h"
namespace pbrpc{
    class TinyPBCoder : public AbstractCoder{
        public:
        //将messge中的对象转化为字节流，写入到buffer
            void encode(std::vector<AbstractProtocol::s_ptr>& messages, Buffer& out_buffer);
        //将buffer里的字节流转化为message对象
            void decode(std::vector<AbstractProtocol::s_ptr>& out_messages, Buffer& buffer);

            ~TinyPBCoder() {}
    };
}