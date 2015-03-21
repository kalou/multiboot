#ifndef _TYPES_H
#define _TYPES_H


typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long u64;

typedef struct _u128 {
    u64 lo;
    u64 hi;
} u128 __attribute__((__aligned__(16)));

typedef struct _start_end {
    u64 start;
    u64 end;
} start_end_t;

#define NULL ((void *) 0)

#define range_bitmask(a, b) (((-1UL) << b) & ((-1UL) >> (63 - a)))

#define set_bit(v, n) (v |= (1UL << n))
#define clear_bit(v, n) (v &= ~(1UL << n))

#define set_range(v, a, b) (v |= range_bitmask(a, b))
#define clear_range(v, a, b) (v &= ~range_bitmask(a, b))

#define bit_set(v, n) ((v & (1UL << n)) == (1UL << n))
#define bit_clear(v, n) (!bit_set(v, n))

#define range_set(v, a, b) ((v & range_bitmask(a, b)) == \
    range_bitmask(a, b))
#define range_clear(v, a, b) ((v & range_bitmask(a, b)) == 0)


#endif
