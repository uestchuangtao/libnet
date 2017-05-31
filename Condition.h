#ifndef CONDITION_H
#define CONDITION_H

#include <assert.h>
#include <pthread.h>

#include <boost/noncopyable.hpp>

#include "Mutex.h"

class Condition:
{
public:
    Condition(MutexLock& mutex)
        :mutex_(mutex)
    {
        assert(!pthread_cond_init(&cond_,NULL));
    }
    ~Condtion()
    {
        assert(!pthread_cond_destroy(&cond_));
    }

    void wait()
    {
        pthread_cond_wait(&cond_,mutex_.getPthreadMutex()); 
    }
    void notify()
    {
        pthread_cond_signal(&cond_,mutex_.getPthreadMutex());
    }

    void notifyAll()
    {
        pthread_cond_broadcast(&cond_,mutex_.getPthreadMutex());
    }


private:
    pthread_cond_t cond_;
    MutexLock& mutex_;
};

#endif


