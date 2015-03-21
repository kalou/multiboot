#ifndef _APIC_H
#define _APIC_H

#include <types.h>
#include <mptable.h>
#include <interrupt.h>

volatile u64 LAPIC;

/* LAPIC registers */
#define ID 0x20
#define VERSION 0x30
#define TASK_PRIO 0x80
#define ARBPR 0x90
#define CPUPR 0xa0
#define EOI 0xb0
#define LDEST 0xd0
#define DST_FMT 0xe0
#define SPIV 0xf0
#define ERROR 0x280
#define ICR 0x300
#define TIMER 0x320
#define PERFC 0x340
#define LINT0 0x350
#define LINT1 0x360
#define ERRV  0x370

#define TIMER_ZERO 0x380
#define TIMER_VAL  0x390
#define TIMER_DIV  0x3e0

#define lapic_read(x) *((volatile u32 *) (LAPIC + x))
#define lapic_write(x, v) { u32 dv; *((volatile u32 *) (LAPIC + x)) = (u32)v; dv = lapic_read(x); dv++;}

void init_apic(void);
void apic_eoi(void);

void install_apic_timer(irq_handler_t handler, int div);

#endif /* _APIC_H */
