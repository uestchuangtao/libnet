#include "Condition.h"

#include <errno.h>
#include <sys/time.h>

bool Condition::waitForSeconds(double seconds)
{
    const int64_t kNSecondsPerSecond= 1e9;
    struct timeval now;
    struct timeval expired;
    assert(!gettimeofday(&now,NULL));
    expired.tv_sec =now.tv_sec;
}