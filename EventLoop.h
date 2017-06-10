//
// Created by ht on 17-6-9.
//

#ifndef LIBNET_EVENTLOOP_H
#define LIBNET_EVENTLOOP_H


#include "TimerId.h"
#include "TimeStamp.h"
#include "Mutex.h"

#include <boost/noncopyable.hpp>
#include <boost/function.hpp>
#include <boost/any.hpp>
#include <boost/scoped_ptr.hpp>

#include <vector>
#include <stdint.h>

class Poller;
class TimeQueue;
class Channel;

class EventLoop :boost::noncopyable {

public:
    typedef boost::function<void()> Functor;
    typedef boost::function<void()> TimerCallback;

    EventLoop();

    ~EventLoop();

    void loop();

    void quit();

    TimeStamp pollReturnTime() const
    {
        return pollReturnTime_;
    }

    //run callback immediately in the loop thread
    void runInLoop(const Functor& cb);

    //Queues callback int the loop thread
    void queueInLoop(const Funtor& cb);

    size_t queueSize() const;

    int64_t iteration() const
    {
        return iteration_;
    }

    TimerId runAt(const TimeStamp& time, const TimerCallback& cb);

    TimerId runAfter(double iterval, const TimerCallback& cb);

    TimerId runEvery(double iterval, const TimerCallback&cb);

    void Cancel(TimerId timerId);

    void wakeup();

    void updateChannel(Channel* channel);

    void removeChannel(Channel* channel);

    void hasChannel(Channel* channel);

    void assertInLoopThread()
    {
        if(!isInLoopThread())
        {
            abortNotInLoopThread();
        }
    }
    bool isInLoopThread() const
    {
        return threadId_ == CurrentThread::tid();
    }

    bool eventHandling() const
    {
        return eventHanding_;
    }

    void setContext(const boost::any& context)
    {
        context_ = context;
    }

    const boost::any& getContext() const
    {
        return context_;
    }

    boost::any* getMutableContext()
    {
        return &context_;
    }

    static EventLoop* getEventLoopOfCurrentThread();

private:
    void abortNotInLoopThread();
    void handleRead();
    void doPendingFunctors();
    void printActiveChannels() const;

    typedef std::vector<Channel*> ChannelList;

    bool looping_;
    bool quit_;
    bool eventHanding_;
    bool callingPendingFunctors_;
    int64_t iteration_;  // TODO: for what?
    const pid_t threadId_;
    TimeStamp pollReturnTime_;
    boost::scoped_ptr<Poller> poller_;
    boost::scoped_ptr<TimerQueue> timerQueue_;

    int wakeupFd_; //TODO: ???
    boost::scoped_ptr<Channel> wakeupChannel_;//TODO: ???

    boost::any context_;

    ChannelList activeChannels_;
    Channel* currentActiveChannel_;

    mutable MutexLock mutex_;
    std::vector<Functor> pendingFunctors_;


};


#endif //LIBNET_EVENTLOOP_H
