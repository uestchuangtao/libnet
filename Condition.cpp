#include "Condition.h"

#include <errno.h>
#include <stdint.h>
#include <time.h>

bool Condition::waitForSeconds(double seconds)
{
    const int64_t kNSecondsPerSecond= 1e9;
    struct timespec now;
    clock_gettime(CLOCK_REALTIME,&now);
    int64_t nseconds=static_cast<int64_t>(seconds*kNSecondsPerSecond);
    now.tv_sec =now.tv_sec+nseconds/kNSecondsPerSecond;
    now.tv_nsec= now.tv_nsec+nseconds%kNSecondsPerSecond;
    const timespec expired = now;
    return ETIMEDOUT == pthread_cond_timedwait(&cond_,&mutex_,&now);
}