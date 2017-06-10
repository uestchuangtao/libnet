//
// Created by ht on 17-6-8.
//

#ifndef LIBNET_CHANNEL_H
#define LIBNET_CHANNEL_H

#include "TimeStamp.h"

#include <boost/noncopyable.hpp>
#include <boost/function.hpp>

#include <string>

class EventLoop;

class Channel : boost::noncopyable {
public:
    typedef boost::function<void (TimeStamp)> ReadEventCallback;
    typedef boost::function<void()> EventCallback;

    Channel(EventLoop* loop,int fd);
    ~Channel();

    void setReadCallback(const ReadEventCallback& cb) //TODO: why ReadEventCallback??? TimeStamp
    {
        readCallback_ = cb;
    }

    void setWriteCallback(const EventCallback& cb)
    {
        writeCallback_ = cb;
    }

    void setCloseCallback(const EventCallback &cb)
    {
        closeCallback_ = cb;
    }

    void setErrorCallback(const EventCallback& cb)
    {
        errorCallback_ = cb;
    }

    int fd() const
    {
        return fd_;
    }

    int events() const
    {
        return events_;
    }

    void set_revents(int revt)  //use by pollers
    {
        revents_ = revt;
    }

    bool isNoneEvent() const
    {
        return events_ == kNonEvent;
    }

    void enableReading()
    {
        events_ |= kReadEvent;
        update();
    }

    void enableWriting()
    {
        events_ |= kWriteEvent;
        update();
    }

    void disableReading()
    {
        events_ &= ~kReadEvent;
        update();
    }

    void disableWriting()
    {
        events_ &= ~kWriteEvent;
        update();
    }

    void disableAll()
    {
        events_ = kNonEvent;
        update();
    }

    bool isReading() const
    {
        return events_ & kReadEvent;
    }

    bool isWriting() const
    {
        return events_ & kWriteEvent;
    }

    int index() const
    {
        return index_;
    }

    void set_index(int idex)
    {
        index_ = index;
    }

    std::string eventsToString() const;
    std::string reventsToString() const;

    EventLoop* ownerLoop() const
    {
        return loop_;
    }

    void remove();

    void handleEvent(TimeStamp receiveTime);
private:

    void update();

    void handleEventWithGuard(TimeStamp receiveTime);

    static std::string eventsToString(int fd, int ev);

    static const int kNonEvent;
    static const int kReadEvent;
    static const int kWriteEvent;

    EventLoop* loop_;
    const int fd_;
    int events_;
    int revents_;  //it's the received event types of epoll or poll
    int index_; //used by poller

    bool eventHandling_;
    bool addedToLoop_;

    ReadEventCallback readCallback_;
    EventCallback writeCallback_;
    EventCallback closeCallback_;
    EventCallback errorCallback_;



};


#endif //LIBNET_CHANNEL_H
