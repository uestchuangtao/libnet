//
// Created by ht on 17-6-3.
//

#include "Thread.h"
#include "Exception.h"
#include "CurrentThread.h"
#include "Timestamp.h"

#include <boost/weak_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/static_assert.hpp>
#include <boost/type_traits/is_same.hpp>

#include <assert.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <stdio.h>  //snprintf
#include <time.h> //nanosleep

namespace CurrentThread {
    __thread int t_cachedTid = 0;
    __thread char t_tidString[32];
    __thread int t_tidStringLength = 6;
    __thread const char* t_threadName = "uknown";
    const bool sameType = boost::is_same<int,pid_t>::value;
    BOOST_STATIC_ASSERT(sameType);
}

namespace detail {
    pid_t gettid()
    {
        return static_cast<pid_t>(::syscall(SYS_gettid));
    }

    void afterFork()
    {
        CurrentThread::t_cachedTid = 0;
        CurrentThread::t_threadName = "main";
        CurrentThread::tid();
    }

    class ThreadNameInitializer {
    public:
        ThreadNameInitializer()
        {
            CurrentThread::t_threadName = "main";
            CurrentThread::tid();
            pthread_atfork(NULL, NULL, &afterFork);
        }
    };

    ThreadNameInitializer init;  //excute once


    struct ThreadData {
        typedef Thread::ThreadFunc ThreadFunc;
        ThreadFunc func_;
        std::string name_;
        boost::weak_ptr<pid_t> wkTid_;

        ThreadData(const ThreadFunc &func, std::string &str, const boost::shared_ptr<pid_t> tid)
                : func_(func),
                  name_(str),
                  wkTid_(tid)
        {

        }

        void runInThread()
        {
            pid_t tid = CurrentThread::tid();
            boost::shared_ptr<pid_t> ptid = wkTid_.lock();
            if(ptid){
                *ptid=tid;
                ptid.reset();
            }
            try {
                func_();
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

    };

    void *startThread(void *obj)
    {

        ThreadData *data = static_cast<ThreadData *>(obj);
        data->runInThread();
        delete data;
        return NULL;
    }

}

void CurrentThread::cachedTid()
{
    if(t_cachedTid == 0)
    {
        t_cachedTid = detail::gettid();
        snprintf(t_tidString,sizeof(t_tidString),"%5d",t_cachedTid);
    }
}

bool CurrentThread::isMainThread()
{
    return ::getpid() == tid();
}

void CurrentThread::sleepUsec(int64_t usec)
{
    struct timespec ts = { 0, 0 };
    ts.tv_sec = static_cast<time_t>(usec / Timestamp::kMicroSecondsPerSecond);
    ts.tv_nsec = static_cast<long> (usec % Timestamp::kMicroSecondsPerSecond * 1000);
    ::nanosleep(&ts,NULL);
}

Thread::Thread(const ThreadFunc &func, const std::string &str)
        : func_(func),
          name_(str),
          pthreadId_(0),
          tid_(new pid_t(0)),
          started_(false),
          joined_(false)
{
    setDefaultName();
}

AtomicInt32 Thread::numCreated_;

Thread::~Thread()
{
    if (joined_ && !started_)
    {
        pthread_detach(pthreadId_);
    }
}

void Thread::setDefaultName()
{
    numCreated_.increment();
    int num = numCreated_.get();
    if (name_.empty())
    {
        char buf[32];
        snprintf(buf, sizeof(buf), "Thread%d", num);
        name_ = buf;
    }
}

void Thread::start()
{
    assert(!started_);
    started_ = true;
    detail::ThreadData *data = new detail::ThreadData(func_, name_, tid_);
    if (pthread_create(&pthreadId_, NULL, &detail::startThread, data)) {
        started_ = false;
        //TODO: LOG_SYS pthread_create error
    }
}

int Thread::join()
{
    assert(started_);
    assert(!joined_);
    joined_ = true;
    return pthread_join(pthreadId_, NULL);
}
