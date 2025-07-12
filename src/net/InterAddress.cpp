#include "src/net/InterAddress.h"

namespace pbrpc{
    InterAddress::InterAddress(const std::string& ip, int port){
        addr_.sin_family = AF_INET;
        addr_.sin_addr.s_addr = inet_addr(ip.c_str());
        addr_.sin_port = htons(port);
    }
    const char* InterAddress::getIp() const{
        return inet_ntoa(addr_.sin_addr);
    }
    uint16_t InterAddress::getPort() const{
        return ntohs(addr_.sin_port);
    }
    sockaddr* InterAddress::getAddr() const{
        return (sockaddr*)&addr_;
    }
}
