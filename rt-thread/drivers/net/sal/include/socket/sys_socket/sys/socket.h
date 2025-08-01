/**
 * RT-Thread RuiChing
 *
 * COPYRIGHT (C) 2024-2025 Shanghai Real-Thread Electronic Technology Co., Ltd.
 * All rights reserved.
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution.
 */
#ifndef __SYS_SOCKET_H__
#define __SYS_SOCKET_H__

#include <rtthread.h>
#include <sal_socket.h>
#ifdef SAL_USING_TLS
#include <sal_tls.h>
#endif /* SAL_USING_TLS */
#include <kpi.h>

#ifdef SAL_USING_POSIX
typedef int (*__kpi_accept)(int s, struct sockaddr *addr, socklen_t *addrlen);
typedef int (*__kpi_bind)(int s, const struct sockaddr *name, socklen_t namelen);
typedef int (*__kpi_shutdown)(int s, int how);
typedef int (*__kpi_getpeername)(int s, struct sockaddr *name, socklen_t *namelen);
typedef int (*__kpi_getsockname)(int s, struct sockaddr *name, socklen_t *namelen);
typedef int (*__kpi_getsockopt)(int s, int level, int optname, void *optval,
    socklen_t *optlen);
typedef int (*__kpi_setsockopt)(int s, int level, int optname, const void *optval,
    socklen_t optlen);
typedef int (*__kpi_connect)(int s, const struct sockaddr *name, socklen_t namelen);
typedef int (*__kpi_listen)(int s, int backlog);
typedef int (*__kpi_recv)(int s, void *mem, size_t len, int flags);
typedef int (*__kpi_recvfrom)(int s, void *mem, size_t len, int flags,
    struct sockaddr *from, socklen_t *fromlen);
typedef int (*__kpi_recvmsg)(int s, struct msghdr *message, int flags);
typedef int (*__kpi_sendmsg)(int s, const struct msghdr *message, int flags);
typedef int (*__kpi_send)(int s, const void *dataptr, size_t size, int flags);
typedef int (*__kpi_sendto)(int s, const void *dataptr, size_t size, int flags,
    const struct sockaddr *to, socklen_t tolen);
typedef int (*__kpi_socket)(int domain, int type, int protocol);
typedef int (*__kpi_closesocket)(int s);
typedef int (*__kpi_ioctlsocket)(int s, long cmd, void *arg);
typedef int (*__kpi_socketpair)(int domain, int type, int protocol, int *fds);
#else
#define accept(s, addr, addrlen)                           sal_accept(s, addr, addrlen)
#define bind(s, name, namelen)                             sal_bind(s, name, namelen)
#define shutdown(s, how)                                   sal_shutdown(s, how)
#define getpeername(s, name, namelen)                      sal_getpeername(s, name, namelen)
#define getsockname(s, name, namelen)                      sal_getsockname(s, name, namelen)
#define getsockopt(s, level, optname, optval, optlen)      sal_getsockopt(s, level, optname, optval, optlen)
#define setsockopt(s, level, optname, optval, optlen)      sal_setsockopt(s, level, optname, optval, optlen)
#define connect(s, name, namelen)                          sal_connect(s, name, namelen)
#define listen(s, backlog)                                 sal_listen(s, backlog)
#define recv(s, mem, len, flags)                           sal_recvfrom(s, mem, len, flags, NULL, NULL)
#define recvfrom(s, mem, len, flags, from, fromlen)        sal_recvfrom(s, mem, len, flags, from, fromlen)
#define recvmsg(s, message, flags)                         sal_recvmsg(s, message, flags)
#define send(s, dataptr, size, flags)                      sal_sendto(s, dataptr, size, flags, NULL, NULL)
#define sendto(s, dataptr, size, flags, to, tolen)         sal_sendto(s, dataptr, size, flags, to, tolen)
#define sendmsg(s, message, flags)                         sal_sendmsg(s, message, flags)
#define socket(domain, type, protocol)                     sal_socket(domain, type, protocol)
#define socketpair(domain, type, protocol, fds)            sal_socketpair(domain, type, protocol, fds)
#define closesocket(s)                                     sal_closesocket(s)
#define ioctlsocket(s, cmd, arg)                           sal_ioctlsocket(s, cmd, arg)
#endif /* SAL_USING_POSIX */

KPI_EXTERN(accept);
KPI_EXTERN(bind);
KPI_EXTERN(shutdown);
KPI_EXTERN(getpeername);
KPI_EXTERN(getsockname);
KPI_EXTERN(getsockopt);
KPI_EXTERN(setsockopt);
KPI_EXTERN(connect);
KPI_EXTERN(listen);
KPI_EXTERN(recv);
KPI_EXTERN(recvfrom);
KPI_EXTERN(recvmsg);
KPI_EXTERN(sendmsg);
KPI_EXTERN(send);
KPI_EXTERN(sendto);
KPI_EXTERN(socket);
KPI_EXTERN(closesocket);
KPI_EXTERN(ioctlsocket);
KPI_EXTERN(socketpair);

#endif /* __SYS_SOCKET_H__ */
