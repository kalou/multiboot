#include <kernel.h>
#include <types.h>
#include <apic.h> /* I only know APIC timers */

static u64 timer_cycles;

void timer_irq(void)
{
    timer_cycles++;
    apic_eoi();
}

void init_timer(void)
{
    timer_cycles = 0;
    install_apic_timer(timer_irq, 1);
}

u64 get_time(void)
{
    return timer_cycles;
}
