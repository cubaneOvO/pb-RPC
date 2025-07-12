#pragma once
#include <map>
#include "src/net/Channel.h"
#include "src/common/TimeEvent.h"
#include "src/common/Config.h"
#include "src/common/log.h"
#include "src/net/EventLoop.h"
#include <mutex>
#include <functional>
namespace pbrpc
{
    class EventLoop;
    class Time
    {
    public:
        Time(EventLoop* el);

        ~Time();
        void addTimerEvent(TimeEvent::s_ptr event);

        void deleteTimerEvent(TimeEvent::s_ptr event);

        void onTime(); // timechannel可读回调

    private:
        std::unique_ptr<Channel> timeChannel_;
        // key: arrive—time value: event
        std::multimap<int64_t, TimeEvent::s_ptr> pending_events_;
        std::mutex mutex_;

        void resetArriveTime();
    };
}