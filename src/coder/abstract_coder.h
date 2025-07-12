#pragma once
#include "src/common/Buffer.h"
#include "src/coder/abstract_protocol.h"
#include <vector>

namespace pbrpc{
    class AbstractCoder{
        public:
            virtual void encode(std::vector<AbstractProtocol::s_ptr>&, Buffer&) = 0;
            virtual void decode(std::vector<AbstractProtocol::s_ptr>&, Buffer&) = 0;
            virtual ~AbstractCoder(){}
    };
}