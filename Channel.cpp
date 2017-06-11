//
// Created by ht on 17-6-8.
//

#include "Channel.h"
#include "EventLoop.h"

#include <poll.h>
#include <assert.h>
#include <sstream> //ostringstream

const int Channel::kNonEvent = 0;
const int Channel::kReadEvent = POLLIN | POLLPRI;
const int Channel::kWriteEvent = POLLOUT;

Channel::Channel(EventLoop *loop, int fd)
    :loop_(loop),
     fd_(fd),
     events_(0),
     revents_(0),
     index_(-1),
     eventHandling_(false),
     addedToLoop_(false)
{

}

Channel::~Channel()
{
    assert(!eventHandling_);
    assert(!addedToLoop_);
    //TODO : check  isInLoopThread()
}

void Channel::remove()
{
    assert(isNoneEvent());
    addedToLoop_ = false;
    loop_->removeChannel(this);
}

void Channel::update()
{
    addedToLoop_ = true;
    loop_->updateChannel(this);
}

void Channel::reset()
{
    index_ = -1;
    events_ = 0;
    revents_ = 0;
    addedToLoop_ = false;
    eventHandling_ = false;
}

void Channel::handleEvent(TimeStamp receiveTime)
{
    // TODO: tied?? guard??
    handleEventWithGuard(receiveTime);

}

void Channel::handleEventWithGuard(TimeStamp receiveTime)
{
    eventHandling_ = true;
    if((revents_ & POLLHUP) && !(revents_ & POLLIN))
    {
        if(closeCallback_)
            closeCallback_();
    }
    if(revents_ & (POLLERR | POLLNVAL))
    {
        if(errorCallback_)
            errorCallback_();
    }
    if(revents_ & (POLLIN | POLLRDHUP || POLLPRI))
    {
        if(readCallback_)
            readCallback_(receiveTime);
    }
    if(revents_ & POLLOUT)
    {
        if(writeCallback_)
            writeCallback_();
    }
    eventHandling_ = false;
}


std::string Channel::eventsToString() const
{
    eventsToString(fd_,events_);
}

std::string Channel::reventsToString() const
{
    eventsToString(fd_,revents_);
}

std::string Channel::eventsToString(int fd, int ev)
{
    std::ostringstream oss;
    out<<fd<<": ";
    if(ev & POLLERR)
    {
        oss<<"ERR ";
    }
    if(ev & POLLHUP)
    {
        oss<<"HUP ";
    }
    if(ev & POLLIN)
    {
        oss<<"IN ";
    }
    if(ev & POLLPRI)
    {
        oss<<"PRI ";
    }
    if(ev & POLLOUT)
    {
        oss<<"OUT ";
    }
    if(ev & POLLNVAL)
    {
        oss<<"NVAL ";
    }

    return oss.str().c_str();

}
