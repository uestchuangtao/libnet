//
// Created by ht on 17-6-10.
//

#include "EPollPoller.h"
#include "Channel.h"

#include <sys/epoll.h>
#include <assert.h>
#include <errno.h>
#include <strings.h>

namespace
{
    const int kNew = -1;
    const int kAdded = 0;
    const int kDeleted = 2;
}

const int EpollPoller::kInitEventListSize;

const char* EpollPoller::operationToString(int op)
{

}

EPollPoller::EPollPoller(EventLoop *loop)
    :Poller(loop),
     epollfd_(::epoll_create1(EPOLL_CLOEXEC)),
     events_(kInitEventListSize)
{
    if(epollfd_ < 0)
    {
        //TODO: LOG_SYSFATAL<<"EPollPoller::EPollPoller";
    }
}

EPollPoller::~EPollPoller()
{
    ::close(epollfd_);
}

TimeStamp EPollPoller::poll(int timeoutMs, ChannelList *activeChannels)
{
    int numEvents = ::epoll_wait(epollfd_,&*events_.begin(), static_cast<int>(events_.size()),timeoutMs);

    TimeStamp now(TimeStamp::now());

    if(numEvents > 0)
    {
        fillActiveChannels(numEvents, activeChannels);
    }
    else if(numEvents == 0)
    {
        //TODO:: LOG_TRACE << "nothing happended";
    }
    else
    {
        int savedErrno = errno;
        if(savedErrno != EINTR)
        {
            //TODO:LOG_SYSERR << "EPollPoller::poll()";
        }
    }
    return now;

}


void EPollPoller::fillActiveChannels(int numEvents, ChannelList *activeChannels) const
{
    assert(implicit_cast<size_t>(numEvents) <= events_.size());

    for(int i = 0; i < numEvents; ++i)
    {
       Channel* channel = static_cast<Channel*>(events_[i].data.ptr);
       auto it = channels_.find(channel->fd());
        assert(it != channels_.end());
        assert(it->second == channel);
        channel->set_revents(events_[i].events);
        activeChannels->push_back(channel);
    }
}

void EPollPoller::updateChannel(Channel *channel)
{
    Poller::assertInLoopThread();
    const int index = channel->index();
    //TODO: LOG_TRACE << "fd = " << channel->fd();
    if(index == kNew || index == kDeleted)
    {
        // a new one, add with EPOLL_CTL_ADD
        int fd = channel->fd();
        if(index == kNew)
        {
            assert(channels_.find(fd) == channels_.end());
            channels_[fd] = channel;
        }
        else //index == kDeleted
        {
            assert(channels_.find(fd) != channels_.end());
            assert(channels_[fd] == channel);
        }
        channel->set_index(kNew);
        update(EPOLL_CTL_ADD,channel);
    }
    else //index == kNew
    {
        int fd = channel->fd();
        assert(channels_.find(fd) != channels_.end());
        assert(channels_[fd] == channel);
        assert(channel->index() == kAdded);

        if(channel->isNoneEvent())
        {
            updateChanel(EPOLL_CTL_DEL,channel);
            channel->set_index(kDeleted);
        }
        else
        {
            update(EPOLL_CTL_MOD,channel);
        }

    }
}

void EPollPoller::removeChannel(Channel *channel)
{
    Poller::assertInLoopThread();

    assert(channel->isNoneEvent());

    int fd = channel->fd();
    assert(channels_.find(fd) != channels_.end());
    assert(channels_[fd] == channel);

    int index = channel->index();
    assert(index == kAdded || index == kDeleted);

    size_t n = channels_.erase(fd);
    assert(n == 1);

    if(kAdded == index)
    {
        update(EPOLL_CTL_DEL, channel);
    }

    channel->set_index(kNew);

}

void EPollPoller::update(int operation, Channel *channel)
{
    struct epoll_event event;
    bzero(&event, sizeof(event));
    event.events = channel->events();
    event.data.ptr = channel;
    int fd = channel->fd();
    //TODO: LOG_TRACE<< " epoll_ctl op = "<<operationToString(operation) << "fd = "<<fd<<" events = {" << channel->eventsToString() << "}";
    if(::epoll_ctl(epollfd_, operation, fd, &event) < 0)
    {
        //LOG_SYS<<"EPollPoller::epoll_ctl";
    }
}

const char* EPollPoller::operationToString(int op)
{
    switch(op)
    {
        case EPOLL_CTL_MOD:
            return "MOD";
        case EPOLL_CTL_ADD:
            return "ADD";
        case EPOLL_CTL_DEL:
            return "DEL";
        default:
            return "UNKNOWN OPERATION";
    }
}

