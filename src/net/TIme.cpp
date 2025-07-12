#include "src/net/Time.h"
namespace pbrpc
{

    int createTimefd(int64_t timeval = 30)
    {                                                                          // 创建timeFd
        int tfd = timerfd_create(CLOCK_MONOTONIC, TFD_CLOEXEC | TFD_NONBLOCK); // 创建timefd
        struct itimerspec timeout;
        memset(&timeout, 0, sizeof(struct itimerspec));
        timeout.it_value.tv_sec = timeval / 1000;
        timeout.it_value.tv_nsec = (timeval % 1000) * 1000000;
        timerfd_settime(tfd, 0, &timeout, 0);
        return tfd;
    }

    Time::Time(EventLoop *el) : timeChannel_(new Channel(el, createTimefd(CConfig::GetInstance()->timeout_)))
    {
        timeChannel_->setReadcallback(std::bind(&Time::onTime, this));
        timeChannel_->enableReading();
    }

    Time::~Time() {}
    void Time::addTimerEvent(TimeEvent::s_ptr event)
    {
        bool is_reset_timerfd = false;

        {
            std::unique_lock<std::mutex> lock(mutex_);
            if (pending_events_.empty())
            {
                is_reset_timerfd = true;
            }
            else
            {
                auto it = pending_events_.begin();
                if ((*it).second->getArriveTime() > event->getArriveTime())
                {
                    is_reset_timerfd = true;
                }
            }
            pending_events_.emplace(event->getArriveTime(), event);
        }

        if (is_reset_timerfd)
        {
            resetArriveTime();
        }
    }

    void Time::deleteTimerEvent(TimeEvent::s_ptr event)
    {
        event->setCancled(true);

        {
            std::unique_lock<std::mutex> lock(mutex_);

            auto begin = pending_events_.lower_bound(event->getArriveTime());
            auto end = pending_events_.upper_bound(event->getArriveTime());

            auto it = begin;
            for (it = begin; it != end; ++it)
            {
                if (it->second == event)
                {
                    break;
                }
            }

            if (it != end)
            {
                pending_events_.erase(it);
            }
        }

        XLOG_DEBUG("success delete TimerEvent at arrive time {}", event->getArriveTime());
    }

    void Time::onTime()
    {
        char buf[8];
        while (true)
        {
            if ((read(timeChannel_->getFd(), buf, 8) == -1) && errno == EAGAIN)
            {
                break;
            }
        }

        timeval val;
        gettimeofday(&val, NULL);
        int64_t now = val.tv_sec * 1000 + val.tv_usec / 1000;

        std::vector<TimeEvent::s_ptr> tmps;                           // 超时事件
        std::vector<std::pair<int64_t, std::function<void()>>> tasks; // 超时事件的<到达时间，callback>

        {
            std::unique_lock<std::mutex> lock(mutex_);
            auto it = pending_events_.begin();
            for (; it != pending_events_.end(); ++it)
            {
                if (it->first <= now)
                { // 超时
                    if (!(it->second->isCancled()))
                    { // 未被取消
                        tmps.push_back(it->second);
                        tasks.push_back(std::make_pair(it->second->getArriveTime(), it->second->getCallBack()));
                    }
                }
                else
                {
                    break;
                }
            }

            pending_events_.erase(pending_events_.begin(), it);
        }

        // 需要把重复的Event 再次添加进去
        for (auto i = tmps.begin(); i != tmps.end(); ++i)
        {
            if ((*i)->isRepeated()) // 循环监听
            {
                // 调整 arriveTime
                (*i)->resetArriveTime();
                addTimerEvent(*i);
            }
        }

        resetArriveTime();

        for (auto i : tasks) // 执行回调
        {
            if (i.second)
            {
                i.second();
            }
        }

    } // timechannel可读回调

    void Time::resetArriveTime()
    {

        int64_t inteval = 0;
        {
            std::unique_lock<std::mutex> lock(mutex_);
            auto tmp = pending_events_;

            if (tmp.size() == 0)
            {
                return;
            }

            timeval val;
            gettimeofday(&val, NULL);
            int64_t now = val.tv_sec * 1000 + val.tv_usec / 1000;

            auto it = tmp.begin();

            if (it->second->getArriveTime() > now)
            {
                inteval = it->second->getArriveTime() - now;
            }
            else
            {
                inteval = CConfig::GetInstance()->timeout_;
            }
        }

        struct itimerspec timeout;
        memset(&timeout, 0, sizeof(struct itimerspec));
        timeout.it_value.tv_sec = inteval / 1000;
        timeout.it_value.tv_nsec = (inteval % 1000) * 1000000;
        int rt = timerfd_settime(timeChannel_->getFd(), 0, &timeout, 0);
        if (rt != 0)
        {
            XLOG_ERROR("timerfd_settime error, errno={}, error={}", errno, strerror(errno));
        }
    }
}