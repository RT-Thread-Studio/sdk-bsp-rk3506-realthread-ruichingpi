#ifndef __RT_TIME__
#define __RT_TIME__

#include <rtthread.h>
#include <sys/time.h>

char* rt_ctime(const time_t* tim_p);
time_t rt_time(time_t* t);

#endif /* __RT_TIME__ */
