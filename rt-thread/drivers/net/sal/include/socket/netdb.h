/**
 * RT-Thread RuiChing
 *
 * COPYRIGHT (C) 2024-2025 Shanghai Real-Thread Electronic Technology Co., Ltd.
 * All rights reserved.
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution.
 */
#ifndef __NETDB_H__
#define __NETDB_H__

#include <stddef.h>
#include <sal_netdb.h>
#include <kpi.h>

typedef struct hostent *(*__kpi_gethostbyname)(const char *name);
typedef int (*__kpi_gethostbyname_r)(const char *name, struct hostent *ret,
    char *buf, size_t buflen, struct hostent **result, int *h_errnop);
typedef void (*__kpi_freeaddrinfo)(struct addrinfo *ai);
typedef int (*__kpi_getaddrinfo)(const char *nodename, const char *servname,
    const struct addrinfo *hints,struct addrinfo **res);

KPI_EXTERN(gethostbyname);
KPI_EXTERN(gethostbyname_r);
KPI_EXTERN(freeaddrinfo);
KPI_EXTERN(getaddrinfo);

#endif /* __NETDB_H__ */
