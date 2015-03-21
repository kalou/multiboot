#include <multiboot.h>
#include <kernel.h>
#include <console.h>

#include <interrupt.h>
#include <apic.h>

#include <timer.h>
#include <memory.h>

#include <mptable.h>

void kernel_entry (unsigned long magic, unsigned long addr)
{
    u64 t;
    multiboot_info_t *mbi = (multiboot_info_t *) addr;

    init_console();

    printk("Magic %x, addr %x\n", magic, addr);
    if (magic != 0x2badb002) {
        panic("Bad multiboot magic, aborting\n");
    }

    init_memory(mbi);
    init_interrupts();
    init_apic();
    init_timer();

    mp_scan();

    t = get_time();
    while (1) {
        if (t != get_time()) {
            printk("time %x\n", get_time());
            t = get_time();
        }
        asm("hlt");
    }
}
