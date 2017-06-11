//
// Created by ht on 17-6-10.
//

#ifndef LIBNET_POLLER_H
#define LIBNET_POLLER_H

#include "EventLoop.h"

#include <boost/noncopyable.hpp>

#include <map>
#include <vector>

#define USE_EPOLL

class Channel;
class TimeStamp;

class Poller : boost::noncopyable {
public:
    typedef std::vector<Channel*> ChannelList;

    Poller(EventLoop* loop);
    virtual ~Poller();

    virtual Timestamp poll(int timeoutMs,ChannelList* activeChannels) = 0;


    virtual void updateChannel(Channel* channel) = 0;
    virtual void removeChannel(Channel* channel) = 0;

    bool hasChannel(Channel* channel);

    void assertInLoopThread() const
    {
        ownerLoop_->assertInLoopThread();
    }

    static Poller* newDefaultPoller(EventLoop* loop);

protected:
    typedef std::map<int,Channel*> ChannelMap;
    ChannelMap channels_;
private:
    EventLoop* ownerLoop_;
};


#endif //LIBNET_POLLER_H
