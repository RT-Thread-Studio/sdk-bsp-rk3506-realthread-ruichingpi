/**
 * RT-Thread RuiChing
 *
 * COPYRIGHT (C) 2024-2025 Shanghai Real-Thread Electronic Technology Co., Ltd.
 * All rights reserved.
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution.
 */
#ifndef __IOCTL_H__
#define __IOCTL_H__

#define _IOC_SIZEBITS 14

#ifndef _IOC_NONE
#define _IOC_NONE 0U
#endif /* _IOC_NONE */

#ifndef _IOC_WRITE
#define _IOC_WRITE 1U
#endif /* _IOC_WRITE */

#ifndef _IOC_READ
#define _IOC_READ 2U
#endif /* _IOC_READ */

/* 
 * | 31 - 30 | 29 - 16 | 15 - 8 | 7 - 0 |
 * |---------|---------|--------|-------|
 * |   dir   |  type   |  nr    | size  |
 */
#define _IOC_DIRSHIFT  30
#define _IOC_TYPESHIFT 16
#define _IOC_NRSHIFT   8
#define _IOC_SIZESHIFT 0

#ifndef _IOC
#define _IOC(dir, type, nr, size)                                              \
    (((dir) << _IOC_DIRSHIFT) | ((type) << _IOC_TYPESHIFT) |                   \
        ((nr) << _IOC_NRSHIFT) | ((size) << _IOC_SIZESHIFT))
#endif /* _IOC */

extern unsigned int __invalid_size_argument_for_IOC;
#define _IOC_TYPECHECK(t)                                                      \
    ((sizeof(t) == sizeof(t[1]) && sizeof(t) < (1 << _IOC_SIZEBITS)) ?         \
            sizeof(t) :                                                        \
            __invalid_size_argument_for_IOC)

#ifndef _IO
#define _IO(type, nr) _IOC(_IOC_NONE, (type), (nr), 0)
#endif /* _IO */

#ifndef _IOR
#define _IOR(type, nr, size)                                                   \
    _IOC(_IOC_READ, (type), (nr), (_IOC_TYPECHECK(size)))
#endif /* _IOR */

#ifndef _IOW
#define _IOW(type, nr, size)                                                   \
    _IOC(_IOC_WRITE, (type), (nr), (_IOC_TYPECHECK(size)))
#endif /* _IOW */

#ifndef _IOWR
#define _IOWR(type, nr, size)                                                  \
    _IOC((_IOC_READ | _IOC_WRITE), (type), (nr), (_IOC_TYPECHECK(size)))
#endif /* _IOWR */

#endif /* __IOCTL_H__ */
