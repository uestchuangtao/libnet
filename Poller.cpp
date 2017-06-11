//
// Created by ht on 17-6-10.
//

#include "Poller.h"
#include "Channel.h"
#include "PollPoller.h"
#include "EPollPoller.h"


Poller* Poller::newDefaultPoller(EventLoop *loop)
{
#ifdef USE_EPOLL
    return new EPollPoller(loop);
#else
    return new PollPoller(loop);
#endif
}

Poller::Poller(EventLoop *loop)
    :ownerLoop_(loop)
{

}

Poller::~Poller()
{

}

bool Poller::hasChannel(Channel *channel)
{
    assertInLoopThread();

    int fd = channel->fd();
    auto it = channels_.find(fd);
    return it != channels_.end() && it->second == channel;
}
