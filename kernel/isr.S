.code64

/* Save and Restore C-passing args */
.macro SAVE
    push %rdi;
    push %rsi;
    push %rdx;
    push %rcx;
    push %r8;
    push %r9;
    push %r10;
    push %r11;
.endm

.macro RESTORE
    pop %r11;
    pop %r10;
    pop %r9;
    pop %r8;
    pop %rcx;
    pop %rdx;
    pop %rsi;
    pop %rdi;
.endm

/* Used to dispatch errcode exceptions */
.macro ERR_INTRGATE num
.globl isr_int_\num
isr_int_\num:
    SAVE
    mov $\num, %rdi
    mov 64(%rsp), %rsi /* errcode */
    lea 72(%rsp), %rdx /* struct idt_regs */
    call interrupt_gate
    RESTORE
    iretq
.endm

/* Used to dispatch no-errcode exceptions */
.macro INTRGATE num
.globl isr_int_\num
isr_int_\num:
    SAVE
    mov $\num, %rdi
    xor %rsi, %rsi      /* errcode */
    lea 64(%rsp), %rdx /* struct idt_regs */
    call interrupt_gate
    RESTORE
    iretq
.endm

/* Used to dispatch IRQs */
.macro IRQGATE num
.globl isr_int_\num
isr_int_\num:
    SAVE
    mov $\num, %rdi
    call irq_gate
    RESTORE
    iretq
.endm

INTRGATE 0
INTRGATE 1
INTRGATE 2
INTRGATE 3
INTRGATE 4
INTRGATE 5
INTRGATE 6
INTRGATE 7
ERR_INTRGATE 8
INTRGATE 9
ERR_INTRGATE 10
ERR_INTRGATE 11
ERR_INTRGATE 12
ERR_INTRGATE 13
ERR_INTRGATE 14
/* 15 reserved */
INTRGATE 16
ERR_INTRGATE 17
INTRGATE 18
INTRGATE 19
/* 20-31 reserved */

/* User defined are IRQs */
IRQGATE 32
IRQGATE 33
IRQGATE 34
IRQGATE 35
IRQGATE 36
IRQGATE 37
IRQGATE 38
IRQGATE 39
IRQGATE 40
IRQGATE 41
IRQGATE 42
IRQGATE 43
IRQGATE 44
IRQGATE 45
IRQGATE 46
IRQGATE 47
IRQGATE 48
