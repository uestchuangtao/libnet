//
// Created by ht on 17-6-1.
//

#ifndef LIBNET_BLOCKINGQUEUE_H
#define LIBNET_BLOCKINGQUEUE_H

#include "Mutex.h"
#include "Condition.h"

#include <boost/noncopyable.hpp>

#include <assert.h>
#include <deque>

template<typename T>
class BlockQueue: boost::noncopyable {
public:
    BlockQueue()
            :mutex_(),
             notEmpty_(mutex_),
             queue_()
    {

    }
    T take(){
        MutexLockGuard lock(mutex_);
        while(queue_.empty()){
            notEmpty_.wait();
        }
        assert(!queue_.empty());
        T front(queue_.front());
        queue_.pop_front();
        return front;

    }
    void put(const T &x)
    {
        MutexLockGuard lock(mutex_);
        queue_.push_back(x);
        notEmpty_.notify();
    }
    size_t size() const
    {
        MutexLockGuard lock(mutex_);
        return queue_.size();
    }
private:
    mutable MutexLock mutex_;
    Condition notEmpty_;
    std::deque<T> queue_;
};

#endif //LIBNET_BLOCKINGQUEUE_H
