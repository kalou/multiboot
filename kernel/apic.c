#include <interrupt.h>
#include <kernel.h>
#include <asm.h>
#include <apic.h>

#define APIC_PRESENT (1<<9)
#define APIC_MSR 0x1b

#define APIC_ENABLED (1<<8)

#define APIC_TIMER_PERIODIC (1<<17)

volatile u64 LAPIC;

void spurious(void)
{
    printk("Spurious IRQ\n");
}

void apic_error(void)
{
    printk("APIC ERR IRQ\n");
}

void init_apic(void)
{
    u32 a, b;

    /* Initialize local APIC */
    /* Mask out everything from the PIC */
    outb(0xa1, 0xff);
    outb(0x21, 0xff);

    /* Available since P6: this late, panic is
    an option */
    cpuid2(1, &a, &b);
    if (!(b & APIC_PRESENT)) {
        panic("No APIC per CPUID, fix me\n");
    }

    LAPIC = rdmsr(APIC_MSR) & 0xffff0000;
    printk("LAPIC addr %x\n", LAPIC);
    printk("LAPIC id %x\n", lapic_read(ID));

    a = install_irq(spurious);
    lapic_write(SPIV, a | APIC_ENABLED);

    a = install_irq(apic_error);
    lapic_write(ERRV, a);
}

void install_apic_timer(irq_handler_t handler, int div)
{
    u32 nr;
    /* Timer is one-shot to our timer interrupt */
    nr = install_irq(handler);
    printk("Installing periodic APIC timer on IRQ%d\n", nr);
    lapic_write(TIMER, (APIC_TIMER_PERIODIC) | nr);

    lapic_write(TIMER_DIV, div);
    lapic_write(TIMER_ZERO, 0x3fffffff);
}

void install_apic_keyboard(irq_handler_t handler)
{
    u32 nr;
    nr = install_irq(handler);
    printk("Installing keyboard handler on IRQ%d\n", nr);
}

void adjust_apic_timer(int div, int zero)
{
    lapic_write(TIMER_DIV, div);
    lapic_write(TIMER_ZERO, zero);
}

void apic_eoi(void)
{
    lapic_write(EOI, 0);
}
