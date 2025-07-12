#include "src/net/EventLoop.h"
namespace pbrpc
{

    EventLoop::EventLoop()
        : ep_(new Epoll), wakeupFd_(eventfd(0, EFD_NONBLOCK)), wakeChannel_(new Channel(this, wakeupFd_)),
          stop_(false), time_(new Time(this))
    {
        wakeChannel_->setReadcallback(std::bind(&EventLoop::handleWakeup, this));
        wakeChannel_->enableReading();
    }

    EventLoop::~EventLoop()
    {
    }

    void EventLoop::run()
    {
        threadId_ = syscall(SYS_gettid); // 获取事件循环所在线程的id
        // printf("Eventloop::run() thread is %d\n",syscall(SYS_gettid));
        std::vector<Channel *> vec;
        while (!stop_)
        { // 事件循环
            vec = ep_->loop();
            if (vec.size() == 0)
            { // 调用回调函数
                if (epollTimeoutCallback_)
                    epollTimeoutCallback_(this);
            }
            for (auto &ch : vec)
                ch->handleEvent();
        }
    }

    void EventLoop::stop()
    {
        stop_ = true;
        wakeup(); // 唤醒epoll_wait
    }

    void EventLoop::setChannel(Channel *ch)
    {
        ep_->setChannel(ch);
    }

    void EventLoop::removeChannel(Channel *ch)
    {
        ep_->removeChannel(ch);
    }

    void EventLoop::setepollTimeoutCallback(std::function<void(EventLoop *)> fn)
    {
        if (!epollTimeoutCallback_)
            epollTimeoutCallback_ = fn;
    }

    bool EventLoop::isLoopThread()
    {
        return (threadId_ == syscall(SYS_gettid)); // 判断当前线程是否是事件循环线程
    }

    void EventLoop::runInLoop(std::function<void()> fn)
    {
        if (isLoopThread())
        {
            fn();
        }
        else
        {
            queueInLoop(fn);
        }
    }

    void EventLoop::queueInLoop(std::function<void()> fn)
    {
        {
            std::lock_guard<std::mutex> lock(this->taskMutex_); // 给任务队列加锁
            taskqueue_.push(fn);
        }
        wakeup(); // 唤醒IO线程
    }

    void EventLoop::wakeup()
    {
        uint64_t val = 1;
        write(wakeupFd_, &val, sizeof(val)); // 随便写点东西去唤醒IO线程
    }

    void EventLoop::handleWakeup()
    {
        uint64_t val;
        int ret = read(wakeupFd_, &val, sizeof(val)); // 尝试读取判断是否有发送任务
        if (ret != -1)
        {
            std::function<void()> fn;
            std::lock_guard<std::mutex> lock(this->taskMutex_); // 给任务队列加锁
            while (taskqueue_.size() > 0)
            { // 执行队列中全部任务
                fn = std::move(taskqueue_.front());
                taskqueue_.pop();
                fn();
            }
        }
    }

    void EventLoop::addTimerEvent(TimeEvent::s_ptr event)
    {
        time_->addTimerEvent(event);
    }
}
