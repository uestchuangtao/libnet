#ifndef LIBNET_MUTEX_H
#define LIBNET_MUTEX_H

#include <boost/noncopyable.hpp>

#include <assert.h>
#include <pthread.h>




class MutexLock:boost::noncopyable {
public:
    MutexLock(){
        assert(!pthread_mutex_init(&mutex_,NULL));
    }
    ~MutexLock(){
        assert(!pthread_mutex_destroy(&mutex_));
    }

    void lock(){
        pthread_mutex_lock(&mutex_);
    }
    void unlock(){
        pthread_mutex_unlock(&mutex_);
    }
    
    pthread_mutex_t *getPthreadMutex()
    {
        return &mutex_;
    }

private:
    pthread_mutex_t mutex_;
};

class MutexLockGuard:boost::noncopyable
{
public:
    explicit MutexLockGuard(MutexLock& mutex)
        :mutex_(mutex)
    {
        mutex_.lock();
    }
    ~MutexLockGuard()
    {
        mutex_.unlock();
    }

private:
    MutexLock& mutex_;
};

#endif

/* -------------------------------------------------
    Prevent misuse like:
    MutexLockGuard(mutex);
    A tempory object doesn't hold the lock for long
    -----------------------------------------------*/
//#define MutexLockGuard(x) error "Missing guard object name"