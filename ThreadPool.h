//
// Created by ht on 17-6-4.
//

#ifndef LIBNET_THREADPOOL_H
#define LIBNET_THREADPOOL_H

#include "Mutex.h"
#include "Condition.h"
#include "Thread.h"

#include <boost/noncopyable.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/function.hpp>

#include <deque>
#include <string>

class ThreadPool : boost::noncopyable {
public:
    typedef boost::function<void()> Task;
    explicit ThreadPool(const std::string& str=std::string("ThreadPool"));
    ~ThreadPool();

    void setMaxQueueSize(int maxSize){
        maxQueueSize_= maxSize;
    }

    void setThreadInitCallback(const Task& cb){
        threadInitCallback_ = cb;
    }

    const std::string& name() const {
        return name_;
    }

    size_t queueSize() const;

    void start(int numThreads);

    void stop();

    void run(const Task& task);

private:
    bool isFull() const;
    void runInThread();
    Task take();

    mutable MutexLock mutex_;
    Condition notFull_;
    Condition notEmpty_;
    std::string name_;
    Task threadInitCallback_;
    std::deque<Task> queue_;
    boost::ptr_vector<Thread> threads_;
    size_t maxQueueSize_;
    bool running_;

};


#endif //LIBNET_THREADPOOL_H
