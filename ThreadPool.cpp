//
// Created by ht on 17-6-4.
//

#include "ThreadPool.h"
#include "Exception.h"

#include <boost/bind.hpp>

#include <assert.h>

ThreadPool::ThreadPool(const std::string &str)
    :mutex_(),
     notEmpty_(mutex_),
     notFull_(mutex_),
     name_(str),
     maxQueueSize_(0),
     running_(false)
{

}

ThreadPool::~ThreadPool()
{
    if(running_){
        stop();
    }
}

size_t ThreadPool::queueSize() const
{
    MutexLockGuard lock(mutex_);
    return queue_.size();
}

void ThreadPool::start(int numThreads)
{
    assert(!running_);
    assert(threads_.empty());

    threads_.reserve(numThreads);
    running_ = true;

    for(int i=0; i < numThreads; ++i){
        char id[32];
        snprintf(id,sizeof(id),"%d",i+1);
        threads_.push_back(new Thread(boost::bind(&runInThread,this),name_ + id));
        threads_[i].start();
    }

    if(numThreads==0 && threadInitCallback_){
        threadInitCallback_();
    }
}

void ThreadPool::stop()
{
    {
        MutexLockGuard lock(mutex_);
        running_ = false;

        notEmpty_.notifyAll();  //TODO: wake up all thread block in take task, and then thread exit loop
    }

    for_each(threads_.begin(),threads_.end(), boost::bind(&Thread::join,_1));
}

void ThreadPool::run(const Task &task)
{
    if(threads_.empty()){
        task();
    }
    else {
        MutexLockGuard lock(mutex_);
        while (isFull()) {
            notFull_.wait();
        }
        assert(!isFull());
        queue_.push_back(task);
        notEmpty_.notify();
    }
}

bool ThreadPool::isFull() const
{
    MutexLockGuard lock(mutex_);
    return maxQueueSize_ > 0 && queue_.size() >= maxQueueSize_;
}

void ThreadPool::runInThread()
{
    try{
        //TODO: threadInitCallback  what to do?
        if(threadInitCallback_)
        {
            threadInitCallback_();
        }

        while(running_){
            Task task(take());
            if(task)  //TODO: why judge task is null?  take return null,wakeup by stop
            {
                task();
            }
        }
    }
    catch (const Exception &ex) {
        fprintf(stderr, "reason:%s\n", ex.what());
        abort();
    }
    catch (const std::exception &ex) {
        fprintf(stderr, "reason:%s\n", ex.what());
        abort();
    }
    catch (...) {
        fprintf(stderr, "unknown exception caught in Thread\n");
        throw;
    }

}

ThreadPool::Task ThreadPool::take()
{
    MutexLockGuard lock(mutex_);
    while(queue_.empty() && running_){   //TODO: judge of running_ is important
        notEmpty_.wait();
    }
    //assert(!queue_.empty());  TODO: Why not judge by this?  wakeup by stop,but queue is empty
    Task task;
    if(!queue_.empty()) {
        task = queue_.front();
        queue_.pop_front();
        if (maxQueueSize_ > 0)
            notFull_.notify();
    }
    return task;
}