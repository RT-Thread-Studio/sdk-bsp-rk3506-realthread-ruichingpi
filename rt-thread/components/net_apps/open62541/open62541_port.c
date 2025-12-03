/*
 * RT-Thread Secure
 *
 * Copyright (c) 2021, Shanghai Real-Thread Electronic Technology Co., Ltd.
 *
 * All rights reserved.
 */
#include "open62541_port.h"
#include "open62541.h"
#include <rtthread.h>
#include <rtconfig.h>

/**
 *
 *  ua_clock
 *
 */

UA_DateTime UA_DateTime_now(void)
{
    UA_DateTime now;
    now = time(RT_NULL);

    return UA_DATETIME_UNIX_EPOCH + now * UA_DATETIME_SEC;
}

/* Offset between local time and UTC time */
UA_Int64 UA_DateTime_localTimeUtcOffset(void)
{
    return 0; //TODO: this is set to zero since UA_DateTime_now() is just the local time, and not UTC.
}

/* CPU clock invariant to system time changes. Use only to measure durations,UA_ServerConfig_setCustomHostname
 * not absolute time. */
UA_DateTime UA_DateTime_nowMonotonic(void)
{
    return (((UA_DateTime)rt_tick_get()) * 1000 / RT_TICK_PER_SECOND) * UA_DATETIME_MSEC;
}

/**
 *
 *  ua_architecture_functions
 *
 */
unsigned int UA_socket_set_blocking(UA_SOCKET sockfd)
{
    int on = 0;

    if(ioctl(sockfd, FIONBIO, &on) < 0)
    {
        return UA_STATUSCODE_BADINTERNALERROR;
    }

    return UA_STATUSCODE_GOOD;
}

unsigned int UA_socket_set_nonblocking(UA_SOCKET sockfd)
{
    int on = 0;

    if(ioctl(sockfd, FIONBIO, &on) < 0)
    {
        return UA_STATUSCODE_BADINTERNALERROR;
    }

    return UA_STATUSCODE_GOOD;
}

int gethostname_lwip(char* name, size_t len)
{
    return -1;
}

void UA_initialize_architecture_network(void)
{

}

void UA_deinitialize_architecture_network(void)
{

}

#if UA_IPV6

#if LWIP_VERSION_IS_RELEASE /* lwip_if_nametoindex is not yet released */
unsigned int lwip_if_nametoindex(const char *ifname)
{
    return 1; /* TODO: assume for now that it only has one interface */
}
#endif /* LWIP_VERSION_IS_RELEASE */

#endif /* UA_IPV6 */
