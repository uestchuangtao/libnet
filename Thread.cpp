//
// Created by ht on 17-6-3.
//

#include "Thread.h"
#include "Exception.h"

#include <boost/weak_ptr.hpp>
#include <boost/static_assert.hpp>
#include <boost/type_traits/is_same.hpp>

#include <assert.h>
#include <unistd.h>
#include <sys/syscall.h>


namespace CurrentThread {
    __thread int t_cachedTid = 0;
    __thread char t_tidString[32];
    __thread int t_tidStringLength = 6;
    const bool sameType = boost::is_same<int,pid_t>::value;
    BOOST_STATIC_ASSERT(sameType);
}

namespace detail {
    pid_t gettid() {
        return static_cast<pid_t>(::syscall(SYS_gettid));
    }


    struct ThreadData {
        typedef Thread::ThreadFunc ThreadFunc;
        ThreadFunc func_;
        std::string name_;
        boost::weak_ptr<pid_t> wkTid_;

        ThreadData(const ThreadFunc &func, std::string &str, const boost::shared_ptr<pid_t> &tid)
                : func(func_),
                  name_(str),
                  wkTid_(tid) {

        }

        void runInThread() {
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

    void *startThread(void *obj) {

        ThreadData *data = static_cast<ThreadData *>(obj);
        data->runInThread();
        delete data;
        return NULL;
    }

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

Thread::AtomicInt32 Thread::numCreated_;

Thread::~Thread() {
    if (joined_ && !started_) {
        pthread_detach(pthreadId_);
    }
}

void Thread::setDefaultName() {
    numCreated_.store(numCreated() + 1);
    int num = numCreated_.load();
    if (name_.empty()) {
        char buf[32];
        snprintf(buf, sizeof(buf), "Thread%d", num);
        name = buf;
    }
}

void Thread::start() {
    assert(!started_);
    started_ = true;
    detail::ThreadData *data = new detail::ThreadData(func_, name_, tid_);
    if (pthread_create(&pthreadId_, NULL, &detail::startThread, data)) {
        started_ = false;
        //TODO: LOG_SYS pthread_create error
    }
}

void Thread::join() {
    assert(started_);
    assert(!joined_);
    joined_ = true;
    return pthread_join(pthreadId_, NULL);
}
