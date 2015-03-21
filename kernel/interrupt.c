/* IDT setup */
#include <kernel.h>
#include <asm.h>
#include <interrupt.h>
#include <page.h>

static struct idt_entry idt_vec[255];

void do_gpf(u32 errcode, struct idt_regs *regs)
{
    printk("GPF at %x\n", regs->rip);
    panic(" errcode %d\n", errcode);
}

static int irq_nr;
static irq_handler_t irq_handler[NR_IRQS] = {
    0x00
};


void interrupt_gate(int num, u32 errcode, struct idt_regs *regs)
{
    idt_errcode_t *err = (idt_errcode_t *) &errcode;
    u64 cr2;

    switch(num) {
        case 13:
            do_gpf(errcode, regs);
            break;
        case 14:
            cr2 = get_cr2();
            printk("Page fault %x from %x\n", cr2, regs->rip);
            printk("Errcode is %x\n", errcode);
            trace_page(cr2);
            panic("aborting");
            break;
        default:
            printk("Interrupt %d called from %x\n", num, regs->rip);
            printk(" -- errcode is %x:%d:%d:%d\n", err->ss, err->ti, err->idt, err->ext);
            break;
    }
}

void irq_gate(int num)
{
    if (num < FIRST_IRQ) {
        panic("IRQ mapped < %d!\n", FIRST_IRQ);
    }

    if (irq_handler[num - FIRST_IRQ]) {
        irq_handler[num - FIRST_IRQ]();
    } else {
        printk("Got IRQ%d, no handler\n", num);
    }
}

u32 find_irq_handler(irq_handler_t handler)
{
    int i;
    for (i = 0; i < NR_IRQS; i++)
        if (irq_handler[i] == handler)
            return FIRST_IRQ + i;
    return 0;
}

u32 install_irq(irq_handler_t handler)
{
    int nr;

    nr = find_irq_handler(handler);
    if (nr)
        return nr;

    if (irq_nr >= LAST_IRQ) {
        panic("No more IRQs, fix me\n");
    }

    irq_handler[irq_nr - FIRST_IRQ] = handler;

    return irq_nr++;
}

u32 install_irq_fast(irq_handler_t handler)
{
    SET_IDT(idt_vec, irq_nr++, TRAP, (u64) handler);
    return irq_nr;
}

void init_interrupts(void)
{
    cli();
    memset(idt_vec, '\x00', sizeof(idt_vec));

    INIT_STD_IDT(idt_vec);

    lidt(idt_vec, sizeof(idt_vec));
    irq_nr = FIRST_IRQ;
    sti();
}
