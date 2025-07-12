#pragma once
#include <functional>
#include <memory>
#include <sys/time.h>

namespace pbrpc
{

    class TimeEvent
    {

    public:
        using s_ptr = std::shared_ptr<TimeEvent>;

        TimeEvent(int interval, bool is_repeated, std::function<void()> cb);

        int64_t getArriveTime() const
        {
            return arrive_time_;
        }

        void setCancled(bool value)
        {
            is_cancled_ = value;
        }

        bool isCancled()
        {
            return is_cancled_;
        }

        bool isRepeated()
        {
            return is_repeated_;
        }

        std::function<void()> getCallBack()
        {
            return task_;
        }

        void resetArriveTime();

    private:
        int64_t arrive_time_; // 到期时间(ms)
        int64_t interval_;    // 事件间隔(ms)
        bool is_repeated_;    // 是否要持续监听
        bool is_cancled_;     // 该超时事件是否被取消

        std::function<void()> task_;
    };

}