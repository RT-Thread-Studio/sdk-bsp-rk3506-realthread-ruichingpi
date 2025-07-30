/**
 * RT-Thread RuiChing
 *
 * COPYRIGHT (C) 2024-2025 Shanghai Real-Thread Electronic Technology Co., Ltd.
 * All rights reserved.
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution.
 */
#ifndef __BYTEORDER__
#define __BYTEORDER__

#ifdef __CHECKER__
#define __bitwise __attribute__((bitwise))
#else
#define __bitwise
#endif

typedef rt_uint16_t __bitwise rt_le16_t;
typedef rt_uint32_t __bitwise rt_le32_t;
typedef rt_uint64_t __bitwise rt_le64_t;
typedef rt_uint16_t __bitwise rt_be16_t;
typedef rt_uint32_t __bitwise rt_be32_t;
typedef rt_uint64_t __bitwise rt_be64_t;

/* gcc defines __BIG_ENDIAN__ on big endian targets */
#if defined(__BIG_ENDIAN__) || defined(ARCH_CPU_BIG_ENDIAN)
#define rt_cpu_to_be16(x) (x)
#define rt_cpu_to_be32(x) (x)
#define rt_cpu_to_be64(x) (x)
#define rt_be16_to_cpu(x) (x)
#define rt_be32_to_cpu(x) (x)
#define rt_be64_to_cpu(x) (x)
#define rt_le16_to_cpu(x) __builtin_bswap16(x)
#define rt_le32_to_cpu(x) __builtin_bswap32(x)
#define rt_le64_to_cpu(x) __builtin_bswap64(x)
#define rt_cpu_to_le16(x) __builtin_bswap16(x)
#define rt_cpu_to_le32(x) __builtin_bswap32(x)
#define rt_cpu_to_le64(x) __builtin_bswap64(x)
#else
#define rt_cpu_to_be16(x) __builtin_bswap16(x)
#define rt_cpu_to_be32(x) __builtin_bswap32(x)
#define rt_cpu_to_be64(x) __builtin_bswap64(x)
#define rt_be16_to_cpu(x) __builtin_bswap16(x)
#define rt_be32_to_cpu(x) __builtin_bswap32(x)
#define rt_be64_to_cpu(x) __builtin_bswap64(x)
#define rt_le16_to_cpu(x) (x)
#define rt_le32_to_cpu(x) (x)
#define rt_le64_to_cpu(x) (x)
#define rt_cpu_to_le16(x) (x)
#define rt_cpu_to_le32(x) (x)
#define rt_cpu_to_le64(x) (x)
#endif /* __BIG_ENDIAN__ || ARCH_CPU_BIG_ENDIAN */

#undef __bitwise

#endif /* __BYTEORDER__ */
