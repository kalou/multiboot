/* boot.S - bootstrap the kernel */
/* Copyright (C) 1999, 2001  Free Software Foundation, Inc.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA. */

#define ASM_FILE        1
#include <multiboot.h>
#include <regs.h>

/* C symbol format. HAVE_ASM_USCORE is defined by configure. */
#ifdef HAVE_ASM_USCORE
# define EXT_C(sym)                     _ ## sym
#else
# define EXT_C(sym)                     sym
#endif

/* The size of our stack (16KB). */
#define STACK_SIZE                      0x4000

/* The flags for the Multiboot header. */
/* We pass memory info to boot 64 bits ELF */
# define MULTIBOOT_HEADER_FLAGS         0x00010003

     .text

     .globl  start, _start
start:
_start:
     jmp     multiboot_entry

     /* Align 32 bits boundary. */
     .align  4

     /* Multiboot header. */
multiboot_header:
     /* magic */
     .long   MULTIBOOT_HEADER_MAGIC
     /* flags */
     .long   MULTIBOOT_HEADER_FLAGS
     /* checksum */
     .long   -(MULTIBOOT_HEADER_MAGIC + MULTIBOOT_HEADER_FLAGS)
     /* header_addr */
     .long   multiboot_header
     /* load_addr */
     .long   _start
     /* load_end_addr */
     /* .long   _edata */
     .long   0x00000000
     /* bss_end_addr */
     /* .long   _end */
     .long   0x00000000
     /* entry_addr */
     .long   multiboot_entry

.align 4
.code32

multiboot_entry:
    /* Initialize the stack pointer. */
    movl    $(stack + STACK_SIZE), %esp

    /* Reset EFLAGS. */
    pushl   $0
    popf

    /* Push the pointer to the Multiboot information structure. */
    pushl   $0x00
    pushl   %ebx
    /* Push the magic value. */
    pushl   $0x00
    pushl   %eax

    /* Test for 64bits processor */
    lea     no64_message, %esi
    movl    $0x80000000, %eax
    cpuid
    cmpl    $0x80000001, %eax
    jb      .panic

    movl    $0x80000001, %eax
    cpuid
    testl   $(1 << 29), %edx /* LM */
    jz      .panic

    lea     no64_message, %esi

    /* 64 bits support OK, setup LM */
    movl    %cr0, %eax
    andl    $~CR0_PG, %eax /* Clear paging bit */
    movl    %eax, %cr0

    /* Create initial identity mapping
    *  We map the first GB here. High memory
    *  will be mapped in kernel space.
    */

    /* Erase 3*4096KB at PML4_ADDR */
    xorl    %eax, %eax
    movl    $0x3000, %edi
    movl    1024, %ecx /* 4 bytes per stosl */
    cld
    rep stosl

    /* Point PML4[0] to PDPTE */
    movl    $PML4_ADDR, %edi
    movl    $0x2007, (%edi) /* 0x2000 | P|W|U */

    /* Load CR3 with PML4 */
    movl    %edi, %cr3

    /* Point PDPTE[0] to PDE */
    movl    $0x2000, %edi
    movl    $0x3007, (%edi) /* 0x3000 | P|W|U */

    /* Point PDE[0..512] to physical identity */
    movl    $0x3000, %edi
    movl    $512, %ecx /* setup 512 2MB pages */
    xorl    %ebx, %ebx

.pgloop:
    movl    %ebx, %eax
    orl     $3, %eax    /* P|W */
    orl     $4, %eax    /* USER */
    orl     $128, %eax  /* PS=1 (2MB pages) */
    stosl
    xorl    %eax, %eax
    stosl
    addl    $1024*1024*2, %ebx
    loop .pgloop

//.debug:
//   jmp .debug

    /* Now enable PAE and PSE in cr4 */
    movl    %cr4, %eax
    orl     $CR4_PAE, %eax /* PAE */
    movl    %eax, %cr4

    /* Set LM in the EFER MSR */
    movl    $0xc0000080, %ecx
    rdmsr
    orl     $LM_ENABLE, %eax /* LME */
    wrmsr

    /* Setup DS,ES,FS,GS to a valid segment */
    lgdt    .gdt_ptr
    movl    $0x10, %eax
    movl    %eax, %ds
    movl    %eax, %es
    movl    %eax, %fs
    movl    %eax, %gs
    movl    %eax, %ss

    /* Enable paging */
    movl    %cr0, %eax
    orl     $CR0_PG | CR0_PE, %eax /* PG|PE */
    movl    %eax, %cr0

    /* Double check EFER or die */
    movl    $0xc0000080, %ecx /* EFER */
    rdmsr
    testl   $LM_ACTIVE, %eax /* LMA */
    jz .panic

    ljmp    $0x08, $do_call
do_call:
.align 8
.code64
    pop     %rdi
    pop     %rsi
    call    kernel_entry

.panic:
    movl    $0xb8000, %edi
    movl    $80*24*2, %ecx
    xorl    %eax, %eax
    rep     stosw

    movl    $0xb8000, %edi
    orl     $0x00000700, %eax
.msgloop:
    lodsb
    stosw
    cmp     %al, 0x0700
    jnz     .msgloop

.halt:   pause
     jmp     .halt


/* Segment descriptor */
/* BASE(16) LIMIT(16) */
/* 0x0000   0xffff    */
/* BASE(8) G D L AVL LIMIT(4) P DPL(2) S TYPE(4) BASE(8)  */
/* 0x00    1 0 1 1   1111     1  00    1         00000000 */
/*                 (00FF9)                 (type)  (00)*/
/* TYPE for data, r/w: 0011 (3) */
/* TYPE for code, r/x: 1010 (a) */

.code64
.align 8
gdt: /* NIL seg */
    .long   0x00000000
    .long   0x00000000
.gtd_cs:
    .long   0x0000ffff
    .long   0x00bf9a00
.gtd_ds:
    .long   0x0000ffff
    .long   0x00bf9300
.gdt_ptr:
    .align 8
    .short   . - gdt
    .long   gdt


fakegdt:
    .long   0

no64_message:
    .asciz "No 64 bits support, halting"
enable64_failed:
    .asciz "Unable to enable 64 bits LM, halting"

/* Our stack area. */
.comm   stack, STACK_SIZE
