#ifndef _ATOMIC_H
#define _ATOMIC_H

/* Set range if zero - return 0 if failed (set by
 * someone else) */
static inline int atomic_set_range(u64 *v, int a, int b)
{
    u64 old = *v;

    while (range_clear(old, a, b)) {
        old = *v;
        if (cmpxchg(v, old, old | range_bitmask(a, b)))
            return 1;
    }
    return 0;
}

static inline int atomic_set_bit(u64 *v, int nr)
{
    u64 old = *v;

    while (bit_clear(old, nr)) {
        old = *v;
        if (cmpxchg(v, old, old | (1 << nr)))
            return 1;
    }
    return 0;
}


/* Clear range if set - return 0 if failed (cleared by
 * someone else) */
static inline int atomic_clear_range(u64 *v, int a, int b)
{
    u64 old;
    old = *v;
    while (range_set(old, a, b)) {
        old = *v;
        printk("Clear mask %x\n", ~range_bitmask(a, b));
        if (cmpxchg(v, old, old & ~range_bitmask(a, b)))
            return 1; 
        printk("cmpxchg clear failed %x -> %x\n", old, *v);
    }
    return 0;
}

static inline int atomic_clear_bit(u64 *v, int nr)
{
    u64 old = *v;

    while (bit_set(old, nr)) {
        old = *v;
        if (cmpxchg(v, old, old & ~(1 << nr)))
            return 1;
    }
    return 0;
}

#endif /* _ATOMIC_H */
