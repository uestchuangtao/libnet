//
// Created by ht on 17-6-10.
//

#include "Poller.h"
#include "Channel.h"

Poller::Poller(EventLoop *loop)
    :ownerLoop_(loop)
{

}

Poller::~Poller()
{

}

bool Poller::hasChannel(Channel *channel)
{
    int fd = channel->fd();
    auto it = channels_.find(fd);
    return it != channels_.end() && it->second == channel;
}
