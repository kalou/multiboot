#include <asm.h>
#include <kernel.h>
#include <interrupt.h>
#include <console.h>
#include <stdarg.h>

#define REG(a) ({ \
    register unsigned long reg asm(a); \
    reg; \
})

void panic(char *fmt, ...)
{
    unsigned long cr0, cr2, cr3;
    unsigned long flags;
    struct idt_ptr idt;
    va_list ap;


    __asm__ __volatile__ (
        "mov %%cr0, %%rax\n"
        "mov %%rax, %0\n"
        "mov %%cr2, %%rax\n"
        "mov %%rax, %1\n"
        "mov %%cr3, %%rax\n"
        "mov %%rax, %2\n"
        : "=m" (cr0), "=m" (cr2), "=m" (cr3)
        : /* no input */
        : "%rax"
    );

    set_console_attribute(RED | BG(BLUE) | BRIGHT);
    va_start(ap, fmt);
    vprintk(fmt, ap);

    asm("sidt %0" : "=m"(idt));
    printk("IDT is %x (len %x)\n", idt.base, idt.limit);
    printk("CR0: %x\tCR2: %x\n", cr0, cr2);

    __asm__ __volatile__ (
        "pushf\n"
        "pop %%rax\n"
        "mov %%rax, %0\n"
        : "=m" (flags)
        : /* no input */
        : "%rax"
    );

    printk("CR3: %x\tFL: %x\n", flags);

    printk("CS: %x\tDS: %x\n", get_cs(), get_ds());
    printk("ES: %x\tFS: %x\n", get_es(), get_fs());
    printk("GS: %x\tSS: %x\n", get_gs(), get_ss());

    printk("RAX: %x\tRBX: %x\n", REG("rax"), REG("rbx"));
    printk("RCX: %x\tRDX: %x\n", REG("rcx"), REG("rdx"));
    printk("RSI: %x\tRDI: %x\n", REG("rsi"), REG("rdi"));
    printk("RBP: %x\tRSP: %x\n", REG("rbp"), REG("rsp"));
    printk("R8:  %x\tR9:  %x\n", REG("r8"), REG("r9"));
    printk("R10: %x\tR11: %x\n", REG("r10"), REG("r11"));
    printk("R12: %x\tR13: %x\n", REG("r12"), REG("r13"));
    printk("R14: %x\tR15: %x\n", REG("r14"), REG("r15"));

    while(1) {
        asm("pause\n"
        );
    }
}
