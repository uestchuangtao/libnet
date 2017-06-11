//
// Created by ht on 17-6-10.
//

#include "PollPoller.h"
#include "Channel.h"
#include "TimeStamp.h"

#include <poll.h>
#include <assert.h>

PollPoller::PollPoller(EventLoop *loop)
    :Poller(loop)
{

}

PollPoller::~PollPoller()
{

}

TimeStamp PollPoller::poll(int timeoutMs, ChannelList *activeChannels)
{
    TimeStamp now(TimeStamp::now());
    int numEvents = ::poll(&*pollfds_.begin(), pollfds_.size(), timeoutMs);
    if(numEvents > 0)
    {
        fillActiveChannels(numEvents,activeChannels);
    }
    else if(numEvents == 0)
    {
        //LOG_TRACE << "nothing happended";
    }
    else
    {
        //LOG_SYS << "PollPoller::poll()";
    }
    retun now;
}

void PollPoller::updateChannel(Channel *channel)
{
    Poller::assertInLoopThread();
    //TODO: LOG_TRACE << "fd= "<<channel->fd() << "events = "<< channel->events();
    if(channel->index() < 0)
    {
        assert(channels_.find(channel->fd()) == channels_.end());
        struct pollfd pfd;
        pfd.fd = channel->fd();
        pfd.events = static_cast<short>(channel->events());
        pfd.revents = 0;
        pollfds_.push_back(pfd);
        int idx= static_cast<int>(pollfds_.size())-1;
        channel->set_index(idx);
        channels_[pfd.fd] = channel;
    }
    else
    {
        assert(channels_.find(channel->fd()) == channels_.end());
        assert(channels_[channel->fd()] == channel);
        int idx = channel->index();
        assert(0 <= idx && idx < static_cast<int>(pollfds_.size()));
        struct pollfd& pfd = pollfds_[idx];
        assert(pfd.fd == channel->fd() || pfd.fd == -channel->fd()-1);
        pfd.events = static_cast<short>(channel->events());
        pfd.revents = 0;
        if(channel->isNoneEvent())
        {
            //ignore this pollfd
            pfd.fd = - channel->fd() -1;
        }
        //TODO: to reactive this pfd,ht 2017-6-11
        else if(pfd.fd  == -channel->fd()-1)
        {
            pfd.fd = channel->fd();
        }
    }


}

void PollPoller::removeChannel(Channel *channel)
{
    PollPoller::assertInLoopThread();
    //TODO:LOG_TRACE << "fd=" << channel->fd();
    assert(channels_.find(channel->fd()) != channels_.end());
    assert(channels_[channel->fd()] == channel);
    assert(channel->isNoneEvent());
    int idx = channel->fd();
    assert(0<=idx && idx < static_cast<int>(pollfds_.size()));
    struct pollfd &pfd = pollfds_[idx];
    assert(pfd.fd ==  -channel->fd()-1 && pfd.events == channel->events());
    size_t n = channels_.erase(channel->fd());
    assert(n == 1);
    if(implicit_cast<size_t>(idx) == pollfds_.size()-1)
    {
        pollfds_.pop_back();
    }
    else
    {
        int channelAtEnd = pollfds_.back().fd;
        iter_swap(pollfds_.begin()+idx,pollfds_.end()-1);
        if(channelAtEnd < 0)
        {
            channelAtEnd = -channelAtEnd-1;
        }
        channels_[channelAtEnd]->set_index(idx);
        pollfds_.pop_back();
    }

}

void PollPoller::fillActiveChannels(int numEvents, ChannelList *activeChannels) const
{
    for(auto pfd = pollfds_.begin(); pfd != pollfds_.end() && numEvents> 0; ++pfd)
    {
        if(pfd->revents > 0)
        {
            --numEvents;
            auto ch = channels_.find(pfd->fd);
            assert(ch != channels_.end());
            auto channel = ch->second;
            assert(pfd->fd == channel->fd());
            channel->set_revents(pfd->revents);
            activeChannels->push_back(channel);
        }
    }
}
