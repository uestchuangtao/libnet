//
// Created by ht on 17-6-17.
//

#ifndef LIBNET_EVENTLOOPTHREADPOOL_H
#define LIBNET_EVENTLOOPTHREADPOOL_H


#include <boost/function.hpp>
#include <boost/noncopyable.hpp>
#include <boost/ptr_container/ptr_container.hpp>

#include <vector>

class EventLoop;
class EventLoopThread;

class EventLoopThreadPool :boost::noncopyable {
public:
    typedef boost::function<void(EventLoop*)> ThreadInitCallback;

    EventLoopThreadPool(EventLoop * baseLoop, const std::string& nameArg);
    ~EventLoopThreadPool();
    void setThreadNum(int numThreads)
    {
        numThreads_ = numThreads;
    }
    void start(const ThreadInitCallback& cb = ThreadInitCallback());

    EventLoop* getNextLoop();

    EventLoop* getLoopForHash(size_t hashCode);

    std::vector<EventLoop*> getAllLoops();

    bool started() const
    {
        return started_;
    }

    const std::string name() const
    {
        return name_;
    }

private:

    EventLoop* baseLoop_;
    std::string name_;
    bool started_;
    int numThreads_;
    int next_;
    boost::ptr_vector<EventLoopThread> threads_;
    std::vector<EventLoop*> loops_;

};


#endif //LIBNET_EVENTLOOPTHREADPOOL_H
