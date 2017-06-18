//
// Created by ht on 17-6-17.
//

#ifndef LIBNET_EVENTLOOPTHREAD_H
#define LIBNET_EVENTLOOPTHREAD_H


#include "Mutex.h"
#include "Condition.h"
#include "Thread.h"

#include <boost/noncopyable.hpp>

class EventLoop;

class EventLoopThread : boost::noncopyable {
public:
    typedef  boost::function<void(EventLoop*)> ThreadInitCallback;

    EventLoopThread(const ThreadInitCallback& cb= ThreadInitCallback(), const std::string& name=std::string());
    ~EventLoopThread();
    EventLoop* startLoop();

private:
    void threadFunc();

    EventLoop* loop_;
    bool exiting_;
    Thread thread_;
    MutexLock mutex_;
    Condition cond_;
    ThreadInitCallback callback_;
};


#endif //LIBNET_EVENTLOOPTHREAD_H
