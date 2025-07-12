#pragma once
#include "src/net/Epoll.h"
#include "src/common/Config.h"
#include "src/net/Connection.h"
#include "src/net/Time.h"
#include "src/common/TimeEvent.h"
#include <functional>
#include <sys/syscall.h>
#include <memory>
#include <queue>
#include <mutex>
#include <atomic>
#include <sys/eventfd.h>
#include <sys/timerfd.h>//定时器头文件
#include <unordered_map>

namespace pbrpc{
    class Channel;
    class Epoll;
    class TimeEvent;
    class Time;
    class Connection;
    typedef std::shared_ptr<Connection> spConnection;
    
    class EventLoop
    {
    private:
        std::unique_ptr<Epoll> ep_;
        std::function<void(EventLoop*)>epollTimeoutCallback_;//处理epoll_wait超时的回调函数
        pid_t threadId_;//存储从事件循环线程的id，用于区从事件循环线程与Work线程
        std::atomic_bool stop_;
    
        //任务队列，存放work线程处理后的发送任务，避免了两个不同线程共同处理发送时的竞争问题
        std::queue<std::function<void()>> taskqueue_;
        std::mutex taskMutex_;
        int wakeupFd_;
        std::unique_ptr<Channel> wakeChannel_;
    
        //定时器    
        std::unique_ptr<Time> time_;
    public:
        EventLoop();
        ~EventLoop();
        void run();//运行事件循环
        void stop();//停止事件循环
        void setChannel(Channel*);//向epoll注册事件
        void setepollTimeoutCallback(std::function<void(EventLoop*)>);
        void removeChannel(Channel*);//从epoll中删除事件
        
        bool isLoopThread();//判断当前线程是否为事件循环线程
    
        void runInLoop(std::function<void()>);//根据线程类型执行不同操作
        void queueInLoop(std::function<void()>);//入队函数
        void wakeup();//用eventfd唤醒事件循环
        void handleWakeup();

        void addTimerEvent(TimeEvent::s_ptr);

        void newConnection(spConnection);//向unordered_map中加入元素
        void delConnection(spConnection);//从unordered_map中删除元素
    };
}


