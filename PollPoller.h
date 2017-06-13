//
// Created by ht on 17-6-10.
//

#ifndef LIBNET_POLLPOLLER_H
#define LIBNET_POLLPOLLER_H


#include "Poller.h"

#include <poll.h>

class PollPoller : public Poller {
public:
    PollPoller(EventLoop* loop);
    virtual ~PollPoller();

    virtual Timestamp poll(int timeoutMs, ChannelList* activeChannels);
    virtual void updateChannel(Channel* channel);
    virtual void removeChannel(Channel* channel);

private:
    void fillActiveChannels(int numEvents, ChannelList* activeChannels) const;
    typedef std::vector<struct pollfd> PollFdList;
    PollFdList pollfds_;


};


#endif //LIBNET_POLLPOLLER_H
