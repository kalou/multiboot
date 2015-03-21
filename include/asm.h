#ifndef _ASM_H
#define _ASM_H

#include "kernel.h"
#include "types.h"
#include "regs.h"

static inline void outb(unsigned short port, unsigned char val)
{
    asm volatile ( "outb %0, %1" : : "a"(val), "Nd"(port) );
    /* TODO: Is it wrong to use 'N' for the port? It's not a 8-bit constant. */
    /* TODO: Should %1 be %w1? */
    /* TODO: Is there any reason to force the use of eax and edx? */
}

#define _DEFINE_GET_REG(reg) static inline u64 get_##reg(void) \
{ \
    u64 r; \
    asm("mov %%" #reg ", %0" : "=r"(r) : : "%rax"); \
    return r; \
}

_DEFINE_GET_REG(cs);
_DEFINE_GET_REG(ds);
_DEFINE_GET_REG(es);
_DEFINE_GET_REG(fs);
_DEFINE_GET_REG(gs);
_DEFINE_GET_REG(ss);
_DEFINE_GET_REG(cr2);

static inline void cpuid(u32 level, u32 *a, u32 *b, u32 *c, u32 *d)
{
    asm("cpuid" 
        : "=a"(*a),"=b"(*b),"=c"(*c),"=d"(*d) 
        : "0"(level));
}

static inline void cpuid2(u32 level, u32 *a, u32 *d)
{
    asm("cpuid"
        : "=a"(*a),"=d"(*d)
        : "0"(level)
        : "%ebx", "%ecx");
}

static inline u64 rdmsr(u32 reg)
{
    u64 a, d;
    asm("rdmsr"
        : "=d"(d), "=a"(a)
        : "c"(reg));

    return (d << 32) | a;
}

static inline void wrmsr(u32 reg, u64 value)
{
    u32 a, d;
    a = value & 0xffffffff;
    d = (value >> 32) & 0xffffffff;
    asm("wrmsr"
        : /* no output */ : "c"(reg), "a"(a), "d"(d));
}

static inline int cmpxchg(u64 *ptr, u64 old, u64 new)
{
    u64 __old = old;
    __asm__ __volatile__ ("lock; cmpxchgq %2, %1\n"
        : "=a" (__old), "+m" (*ptr)
        : "r" (new), "0" (__old)
        : "memory");

    if (old != __old) {
        printk("changing %x->%x says %x\n", old, new, __old);
    }

    return (old == __old);
}

static inline int cmpxchg16(u128 *ptr, u128 old, u128 new)
{
    u128 __old = { old.hi, old.lo };
    __asm__ __volatile__ ("lock; cmpxchg16 %1\n"
        : "+m" (*ptr), "+d" (__old.hi), "+a" (__old.lo)
        : "c" (new.hi), "b" (new.lo)
        : "cc" );

    return ((__old.hi == old.hi) && (__old.lo == old.lo));
}

static inline u64 ffs(u64 v)
{
    u64 ret = -1;
    asm("bsrq %1, %0"
        : "+r"(ret)
        : "rm" (v));
    return ret+1;
}

static inline u64 lzc(u64 v)
{
    u64 ret = -1;
    asm("lzcntq %1, %0"
        : "=r"(ret)
        : "r" (v));
    return ret;
}

static inline u64 count_bits(u64 v)
{
    u64 ret = 0;
    asm("popcntq %1, %0"
        : "=r"(ret)
        : "r"(v));
    return ret;
}

#endif
