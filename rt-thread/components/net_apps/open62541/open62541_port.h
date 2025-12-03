/* 
 * RT-Thread Secure 
 *  
 * Copyright (c) 2021, Shanghai Real-Thread Electronic Technology Co., Ltd. 
 * 
 * All rights reserved. 
 */
#ifndef __OPEN62541_PORT_H
#define __OPEN62541_PORT_H

#include <rtthread.h>

#define UA_ARCHITECTURE_RTTHREAD

#ifdef UA_ARCHITECTURE_RTTHREAD

#if UA_MULTITHREADING >= 100
#error Multithreading unsupported
#else
#define UA_LOCK_TYPE(mutexName)
#define UA_LOCK_TYPE_POINTER(mutexName)
#define UA_LOCK_INIT(mutexName)
#define UA_LOCK_DESTROY(mutexName)
#define UA_LOCK(mutexName)
#define UA_UNLOCK(mutexName)
#define UA_LOCK_ASSERT(mutexName, num)
#endif /* UA_MULTITHREADING */

#define UA_strncasecmp strncasecmp

//no getifaddr
#undef UA_HAS_GETIFADDR

#ifndef IN6_IS_ADDR_UNSPECIFIED
#define IN6_IS_ADDR_UNSPECIFIED(a) \
        (((const uint32_t *) (a))[0] == 0                                      \
         && ((const uint32_t *) (a))[1] == 0                                      \
         && ((const uint32_t *) (a))[2] == 0                                      \
         && ((const uint32_t *) (a))[3] == 0)
#endif /* IN6_IS_ADDR_UNSPECIFIED */

#ifdef BYTE_ORDER
# undef BYTE_ORDER
#endif

#define UA_sleep_ms(X) rt_thread_delay(X)

#define UA_free rt_free
#define UA_malloc rt_malloc
#define UA_calloc rt_calloc
#define UA_realloc rt_realloc

#ifdef UA_ENABLE_DISCOVERY_SEMAPHORE
# ifndef UA_fileExists
#  define UA_fileExists(X) (0) /* If the system provides it, please define it before */
# endif // UA_fileExists
#endif /* UA_ENABLE_DISCOVERY_SEMAPHORE */

// enable log colors on
#define UA_ENABLE_LOG_COLORS

#include <stdio.h>
#define UA_snprintf rt_snprintf

#define UA_LOG_SOCKET_ERRNO_WRAP(LOG) { \
    char *errno_str = ""; \
    LOG; \
}

#endif /* UA_ARCHITECTURE_RTTHREAD */

#endif /* __OPEN62541_PORT_H */