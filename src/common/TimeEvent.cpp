#include "src/common/TimeEvent.h"
namespace pbrpc{
    TimeEvent::TimeEvent(int interval, bool is_repeated, std::function<void()> cb): interval_(interval),
        is_repeated_(is_repeated),  task_(cb), is_cancled_(false)
    {
        resetArriveTime();
    }
    void TimeEvent::resetArriveTime(){
        timeval val;
        gettimeofday(&val, NULL);
        arrive_time_ = val.tv_sec * 1000 + val.tv_usec / 1000 + interval_;
    }
}