//
// Created by ht on 17-6-10.
//

#ifndef LIBNET_EPOLLPOLLER_H
#define LIBNET_EPOLLPOLLER_H

#include "Poller.h"

struct epoll_event;

class EPollPoller : public Poller {
public:
    EPollPoller(EventLoop* loop);
    virtual ~EPollPoller();
    virtual TimeStamp poll(int timeoutMs, ChannelList* activeChannels);
    virtual void updateChanel(Channel* channel);
    virtual void removeChanel(Channel* channel);

private:
    static const int kInitEventListSize = 16;

    static const char* operationToString(int op);

    void fillActiveChannels(int numEvents, ChannelList *activeChannels) const;

    typedef std::vector<struct epoll_event> EventList;

    void update(int operation, Channel* channel);

    int epollfd_;
    EventList events_;

};


#endif //LIBNET_EPOLLPOLLER_H
