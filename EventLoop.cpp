//
// Created by ht on 17-6-9.
//

#include "EventLoop.h"
#include "Channel.h"
#include "SocketsOps.h"

#include <boost/bind.hpp>

#include <assert.h>
#include <sys/eventfd.h>
#include <algorithm>

namespace {
    __thread EventLoop* t_loopInThisThread = 0;

    const kPollTimeMs = 10000;

    int createEventfd()
    {
        int evtfd = ::eventfd(0, EFD_NONBLOCK|EFD_CLOEXEC);

        if(evtfd < 0)
        {
            //TODO: LOG_SYS<<"failed in eventfd";
            abort();
        }
        return evtfd;
    }
}

EventLoop* EventLoop::getEventLoopOfCurrentThread()
{
    return t_loopInThisThread;
}

EventLoop::EventLoop()
    :looping_(false),
     quit_(false),
     eventHanding_(false),
     callingPendingFunctors_(false),
     iteration_(0),
     threadId_(CurrentThread::tid()),
     poller_(Poller::newDefaultPoller(this)),
     timerQueue_(new TimerQueue(this)),
     wakeupFd_(createEventfd()),
     wakeupChannel_(new Channel(this,wakeupFd_)),
     currentActiveChannel_(NULL)
{
   if(t_loopInThisThread)
   {
       //TODO: LOG_SYS<<"Another EventLoop exists in this thread";
   }
   else
   {
    t_loopInThisThread = this;
   }
    wakeupChannel_->setReadCallback(boost::bind(&EventLoop::handleRead, this));
    wakeupChannel_->enableReading();
}

EventLoop::~EventLoop()
{
    wakeupChannel_->disableAll();
    wakeupChannel_->remove();
    ::close(wakeupFd_);
    t_loopInThisThread = NULL;
}

void EventLoop::loop()
{
    assert(!looping_);
    assertInLoopThread();
    looping_ = true;
    quit_ = false;

    while(!quit_)
    {
        activeChannels_.clear();  //TODO;memory leak???
        pollReturnTime_ = poller_->poll(kPollTimeMs,&activeChannels_);
        ++iteration_;

        //TODO: printActiveChannels

        eventHanding_ = true;

        auto it=activeChannels_.begin();
        while(it!=activeChannels_.end()){
            currentActiveChannel_ = *it;
            currentActiveChannel_->handleEvent(pollReturnTime_);
        }
        currentActiveChannel_ = NULL; //allthing done this cicyle
        eventHanding_ = false;
        doPendingFunctors();
    }

    looping_ = false;

}

void EventLoop::quit()
{
    quit_ = true;

    if(!isInLoopThread())
    {
        wakeup(); //TODO:how to wakeup???
    }
}

void EventLoop::runInLoop(const Functor &cb)
{
    if(isInLoopThread())
    {
        cb();
    }
    else
    {
        queueInLoop(cb);
    }
}

void EventLoop::queueInLoop(const Funtor &cb)
{
    MutexLockGuard lock(mutex_);
    pendingFunctors_.push_back(cb);

    //TODO: what?  for wakeup loop poller->poll
    if(!isInLoopThread() || callingPendingFunctors_)
    {
        wakeup();
    }
}

size_t EventLoop::queueSize() const
{
    MutexLockGuard lock(mutex_);
    return pendingFunctors_.size();
}

void EventLoop::updateChannel(Channel *channel)
{
    assert(channel->ownerLoop() == this);
    assertInLoopThread();
    poller_->updateChannel(channel);
}

void EventLoop::removeChannel(Channel *channel)
{
    assert(channel->ownerLoop() == this);
    assertInLoopThread();

    assert(channel == currentActiveChannel_ || std::find(activeChannels_.begin(),activeChannels_.end(),channel) == activeChannels_.end());

    poller_->removeChannel(channel);
}

bool EventLoop::hasChannel(Channel *channel)
{
    assert(channel->ownerLoop() == this);
    assertInLoopThread();

    poller_->hasChannel(channel);
}

//TODO:
/*
TimerId EventLoop::runAt(const TimeStamp& time, const TimerCallback& cb);

TimerId EventLoop::runAfter(double iterval, const TimerCallback& cb);

TimerId EventLoop::runEvery(double iterval, const TimerCallback&cb);

void EventLoop::Cancel(TimerId timerId);
 */

void EventLoop::abortNotInLoopThread()
{
    //TODO: LOGO_FATAL<<"EventLoop::abortNotInLoopThread";
}

void EventLoop::wakeup()
{
    uint64_t one = 1;
    size_t n = sockets::write(wakeupFd_,&one,sizeof(one));
    if(n != sizeof(one))
    {
        //TODO: LOG_ERROR<<"EventLoop::wakeup() writes "<<n<<"bytes instead of 8";
    }
}

void EventLoop::handleRead()
{
    uint64_t  one;
    size_t n = sockets::read(wakeupFd_,&one,sizeof(one));
    if(n != sizeof(one))
    {
        //TODO: LOG_ERROR<<"EventLoop::handleRead() read "<<n<<"bytes instead of 8";
    }
}

void EventLoop::doPendingFunctors()
{
    callingPendingFunctors_ = true;
    std::vector<Functor> functors;
    {
        MutexLockGuard lock(mutex_);
        functors.swap(pendingFunctors_);
    }
    for(auto cb:functors)
    {
        cb();
    }
    callingPendingFunctors_ = false;
}

void EventLoop::printActiveChannels() const
{
    for(auto ch:activeChannels_)
    {
        //LOG_TRACE<<"{"<<ch->reventsToString()<<"}";
    }
}


