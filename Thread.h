//
// Created by ht on 17-6-3.
//

#ifndef LIBNET_THREAD_H
#define LIBNET_THREAD_H


#include <boost/noncopyable.hpp>
#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>

#include <pthread.h>
#include <stdint.h>

class Thread : boost::noncopyable {
public:
    typedef boost::function<void()> ThreadFunc;
    typedef std::atomic<uint32_t> AtomicInt32;
    explicit  Thread(const ThreadFunc& func,const std::string& name = std::string());

    ~Thread();

    void start();

    int join();

    void setDefaultName();

    bool started() const
    {
        return started_;
    }

    static int numCreated()
    {
        return numCreated_.load();
    }

private:
    std::string name_;
    ThreadFunc func_;
    pthread_t pthreadId_;
    boost::shared_ptr<pid_t> tid_;
    bool started_;
    bool joined_;
    static AtomicInt32 numCreated_;

};


#endif //LIBNET_THREAD_H
