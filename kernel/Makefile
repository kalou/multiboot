OBJS := entry.o console.o printk.o memcpy.o panic.o interrupt.o isr.o apic.o \
    timer.o memory.o page.o mptable.o
LIB := libkernel.a

include ../conf.mk

isr.h: isr.o
	nm isr.o | awk 'BEGIN { printf("#ifndef _ISR_H\n#define _ISR_H\n"); } /isr_/ \
		{ printf("void %s(void);\n", $$NF); } END { printf("#endif /* _ISR_H_ */\n");}' | tee isr.h
