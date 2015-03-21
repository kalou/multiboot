#ifndef _INTERRUPT_H
#define _INTERRUPT_H

#include "types.h"
#include "isr.h"

struct idt_entry
{
    u16 offset15_0;
    u16 segment;
    unsigned ist : 3, zeroes : 5, type : 4, zero: 1, dpl : 2, p : 1;
    u16 offset31_16;
    u32 offset63_32;
    u32 reserved;
} __attribute__((packed));

#define LOW(x) ((unsigned long)(x) & 0xffff)
#define MED(x) (((unsigned long)(x) >> 16) & 0xffff)
#define HIGH(x) (((unsigned long)(x) >> 32) & 0xffffffff)

enum {
    INTERRUPT = 0xe,
    TRAP = 0xf,
    CALL = 0xc,
    TASK = 0x5,
};

#define SET_IDT(table, nr, _type, fn) \
    table[nr].offset15_0 = LOW(fn); \
    table[nr].segment = KERNEL_CS; \
    table[nr].type = _type; \
    table[nr].p = 1; \
    table[nr].offset31_16 = MED(fn); \
    table[nr].offset63_32 = HIGH(fn);

#define SET_IDT_ISR(table, nr, _type) \
    SET_IDT(table, nr, _type, isr_int_##nr);

#define SET_IDT_TRAP(table, nr) SET_IDT_ISR(table, nr, TRAP)
#define SET_IDT_INTR(table, nr) SET_IDT_ISR(table, nr, INTERRUPT)
#define SET_IDT_IRQ(table, nr) SET_IDT_INTR(table, nr)

#define INIT_STD_IDT(table) { \
    SET_IDT_TRAP(table, 0) \
    SET_IDT_TRAP(table, 1) \
    SET_IDT_TRAP(table, 2) \
    SET_IDT_TRAP(table, 3) \
    SET_IDT_TRAP(table, 4) \
    SET_IDT_TRAP(table, 5) \
    SET_IDT_TRAP(table, 6) \
    SET_IDT_TRAP(table, 7) \
    SET_IDT_TRAP(table, 8) \
    SET_IDT_TRAP(table, 9) \
    SET_IDT_TRAP(table, 10) \
    SET_IDT_TRAP(table, 11) \
    SET_IDT_TRAP(table, 12) \
    SET_IDT_TRAP(table, 13) \
    SET_IDT_TRAP(table, 14) \
    SET_IDT_TRAP(table, 16) \
    SET_IDT_TRAP(table, 17) \
    SET_IDT_TRAP(table, 18) \
    SET_IDT_TRAP(table, 19) \
    SET_IDT_IRQ(table, 32) \
    SET_IDT_IRQ(table, 33) \
    SET_IDT_IRQ(table, 34) \
    SET_IDT_IRQ(table, 35) \
    SET_IDT_IRQ(table, 36) \
    SET_IDT_IRQ(table, 37) \
    SET_IDT_IRQ(table, 38) \
    SET_IDT_IRQ(table, 39) \
    SET_IDT_IRQ(table, 40) \
}

struct idt_ptr
{
    u16 limit;
    u64 base;
} __attribute__((packed));

typedef struct _idt_errcode {
    u32 ext : 1, idt : 1, ti : 1, ss : 16, rsvd : 21;
} __attribute__((packed)) idt_errcode_t;

/* Registers, errcode pushed on stack */
struct idt_regs
{
    u64 rip;
    u64 cs;
    u64 flags;
    u64 rsp;
    u64 ss;
} __attribute((packed));

static inline void cli(void)
{
    asm volatile ( "cli" );
}

static inline void sti(void)
{
    asm volatile ( "sti" );
}

static inline void lidt(void *ptr, int limit)
{
    struct idt_ptr idtr;

    idtr.base = (u64) ptr;
    idtr.limit = limit;

    asm volatile ( "lidt (%0)" : /* no out */ : "p"(&idtr) );
}

void init_interrupts(void);

typedef void(*irq_handler_t)(void);
u32 install_irq(irq_handler_t);

#define FIRST_IRQ 32
#define LAST_IRQ  42
#define NR_IRQS (LAST_IRQ - FIRST_IRQ)

#endif
