#pragma once
#include <vector>
#include <functional>
#include "src/net/EventLoop.h"
#include "src/common/ThreadPool.h"
#include "src/common/log.h"
namespace pbrpc
{
    class EventLoopGroup
    {
    public:
        EventLoopGroup(const EventLoopGroup &) = delete;
        EventLoopGroup &operator=(const EventLoopGroup &) = delete;
        virtual ~EventLoopGroup() {
            
        };

        static EventLoopGroup *GetInstance()
        {
            static EventLoopGroup s_instance;
            return &s_instance;
        }

        EventLoop* getEventLoop(int fd){
            return loops_[fd % CConfig::GetInstance()->ioThread_].get();
        }

        void stopLoop(){
            for(auto &loop : loops_)
                loop->stop();
        }

    private:
        std::vector<std::unique_ptr<EventLoop>> loops_;

        EventLoopGroup() {
            
            for(int i = 0, n = CConfig::GetInstance()->ioThread_; i < n; i++){
                loops_.emplace_back(new EventLoop());
                ThreadPool::GetInstance()->addtask(std::bind(&EventLoop::run, loops_[i].get()));
            }
        };
    };
}