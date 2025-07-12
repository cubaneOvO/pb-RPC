#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/epoll.h>
#include <vector>
#include <unistd.h>
#include <assert.h>
#include "src/net/Channel.h"

namespace pbrpc{
    #define MAX_EVENT_NUM 1024 //修改在配置文件中
    class Channel;
    
    class Epoll{
    private:
        int epollfd_ = -1;
        epoll_event events_[MAX_EVENT_NUM];
    public:
        Epoll(/* args */);
        ~Epoll();
        void setChannel(Channel*ch);
        void removeChannel(Channel*ch);
        std::vector<Channel*> loop(int timeout = -1);
    };
}


