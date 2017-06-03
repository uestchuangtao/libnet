//
// Created by ht on 17-6-3.
//

#ifndef LIBNET_BOUNDEDBLOCKINGQUEUE_H
#define LIBNET_BOUNDEDBLOCKINGQUEUE_H

#include <assert.h>

#include <boost/circular_buffer.hpp>
#include <boost/noncopyable.hpp>

#include "Mutex.h"
#include "Condition.h"
template <typename T>
class BoundedBlockingQueue : boost::noncopyable {
public:
    explicit BoundedBlockingQueue(int maxsize)
        :mutex_(),
         notEmpty_(mutex_),
         notFull_(mutex_),
         queue__(maxsize)
    {

    }

    T take()
    {
        MutexLockGuard lock(mutex_);
        while(queue_.empty()){
            notEmpty_.wait();
        }
        assert(!queue_.empty());
        T front(queue_.front());
        queue_.pop_front();
        notFull_.notify();
        return front;
    }

    void put(const T& x)
    {
        MutexLockGuard lock(mutex_);
        while(queue_.full()){
            notFull_.wait();
        }
        assert(!queue_.full());
        queue_.push_back(T);
        notEmpty_.notify();
    }

    size_t size() const
    {
        MutexLockGuard lock(mutex_);
        return queue_.size();
    }

    bool empty() const
    {
        MutexLockGuard lock(mutex_);
        return queue_.empty();
    }

    bool full() const
    {
        MutexLockGuard lock(mutex_);
        return queue_.full();
    }

private:
    mutable MutexLock mutex_;
    Condition notEmpty_;
    Condition notFull_;
    boost::circular_buffer<T> queue_;
};

#endif //LIBNET_BOUNDEDBLOCKINGQUEUE_H
