/************************************************************************
 * Kernel OSAL platform configuration
 ************************************************************************/

#ifndef OSAL_PLATFORM_H
#define OSAL_PLATFORM_H

#define OSAL_PLATFORM_LINUX
#define OSAL_PLATFORM_KERNEL

#if defined(CONFIG_64BIT) || defined(CONFIG_ARCH_X86_64) || defined(__x86_64__) || \
	defined(__amd64__) || defined(CONFIG_ARCH_ARM64) || defined(__aarch64__) || \
	defined(CONFIG_ARCH_RISCV64)
#define OSAL_ARCH_BITS 0x40
#else
#define OSAL_ARCH_BITS 0x20
#endif

#define OSAL_LITTLE_ENDIAN 0x1
#define OSAL_BIG_ENDIAN 0x0
#define OSAL_PACKED __attribute__((packed))
#define OSAL_INLINE static inline __attribute__((always_inline))
#define OSAL_API
#define OSAL_ALIGNED(x) __attribute__((aligned(x)))

#if OSAL_ARCH_BITS == 0x40
typedef unsigned long long osal_ptr_t;
typedef long long osal_sptr_t;
typedef long long osal_atomic_t;
#else
typedef unsigned long osal_ptr_t;
typedef long osal_sptr_t;
typedef int osal_atomic_t;
#endif

#endif /* OSAL_PLATFORM_H */
