//
// Created by ht on 17-6-10.
//

#ifndef LIBNET_EPOLLPOLLER_H
#define LIBNET_EPOLLPOLLER_H

#include "Poller.h"

#include <sys/epoll.h>


class EPollPoller : public Poller {
public:
    EPollPoller(EventLoop* loop);
    virtual ~EPollPoller();
    virtual Timestamp poll(int timeoutMs, ChannelList* activeChannels);

    virtual void updateChannel(Channel* channel);
    virtual void removeChannel(Channel* channel);

private:
    static const int kInitEventListSize;

    typedef std::vector<struct epoll_event> EventList;

    static const char* operationToString(int op);

    void fillActiveChannels(int numEvents, ChannelList *activeChannels) const;

    void update(int operation, Channel* channel);


    int epollfd_;
    EventList events_;

};


#endif //LIBNET_EPOLLPOLLER_H
