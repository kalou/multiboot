#ifndef _REGS_H
#define _REGS_H

/* PML4 fixed address (setup in boot, used in paging) */
#define PML4_ADDR 0x1000

/* EFER */
#define NX_ENABLE (1<<11)
#define LM_ACTIVE (1<<10)
#define LM_ENABLE (1<<8)
#define SC_ENABLE (1)

/* EFLAGS */
#define EFLAG_TF (1<<8) /* Trap flag */
#define EFLAG_IF (1<<9) /* Int enable */
#define EFLAG_IOPL(x) ((x>>12) & 3)
#define EFLAG_NT (1<<14)
#define EFLAG_RF (1<<16)
#define EFLAG_VM (1<<17)
#define EFLAG_VIF (1<<19)
#define EFLAG_VIP (1<<20)
#define EFLAG_ID (1<<21)

/* CR0 */
#define CR0_PG (1<<31)
#define CR0_CD (1<<30)
#define CR0_NW (1<<29)
#define CR0_AM (1<<18)
#define CR0_WP (1<<16)
#define CR0_NE (1<<5)
#define CR0_ET (1<<4)
#define CR0_TS (1<<3)
#define CR0_EM (1<<2)
#define CR0_MP (2)
#define CR0_PE (1)

/* CR3 */
#define CR3_PCD (1<<4)
#define CR3_PWT (1<<3)

/* CR4 */
#define CR4_VME (1)
#define CR4_PVI (2)
#define CR4_TSD (1<<2)
#define CR4_DE (1<<3)
#define CR4_PSE (1<<4)
#define CR4_PAE (1<<5)
#define CR4_MCE (1<<6)
#define CR4_PGE (1<<7)
#define CR4_PCE (1<<8)
#define CR4_OSFXSR (1<<9)
#define CR4_OSXMMEXCPT (1<<10)
#define CR4_VMXE (1<<13)
#define CR4_SMXE (1<<14)
#define CR4_FSGSBASE (1<<16)
#define CR4_PCIDE (1<<17)
#define CR4_OSXSAVE (1<<18)
#define CR4_SMEP (1<<20)

#define CR8_TPL(x) (x&3)

#ifndef ASM_FILE
#include <types.h>

typedef struct _seg_desc {
    u16 limit15_0;
    u16 base15_0;
    u8  base23_16;
    u8  type:4, s:1, dpl:2, p:1;
    u8  limit19_16:4, avl:1, l:1, db:1, g:1;
    u8  base31_24;
} segdesc_t;

#define SET_ADDR(seg, x) { \
    seg.base15_0 = (x & 0xffff); \
    seg.base31_24 = ((x >> 24) & 0xff); \
    seg.base23_16 = ((x >> 16) & 0xff); }

#define SET_LIMIT(seg, x) { \
    seg.limit15_0 = (x & 0xffff); \
    seg.limit19_16 = (x >> 16) & 0xff; }

#define CODE_SEGMENT (1<<3)
#   define CONFORMING (1<<2)
#   define EXREAD     (1<<1)
#define DATA_SEGMENT (0<<3)
#   define WRITE      (1<<1)
#   define EXDOWN     (1<<2)
#define ACCESSED      1

#define MAXPHYADDR 52 /* .. */

#define PAGE_PRESENT (1)
#define PAGE_WRITE (1<<1)
#define PAGE_USER (1<<2)
#define PAGE_PWT (1<<3)
#define PAGE_PCD (1<<4)
#define PAGE_ACCESSED (1<<5)
#define PAGE_DIRTY (1<<6)
#define PAGE_SIZE_SET (1<<7)
#define PAGE_GLOBAL (1<<8)
#define PAGE_PAT (1<<12)
#define PAGE_NX (1UL<<63)

typedef u64 pg_desc_t;

#define pg_desc_addr(x) (x & range_bitmask(MAXPHYADDR, 12))

/* Entry number into PML4/PDPTE/PDE tables */
#define PML4E(addr) (((u64)addr & range_bitmask(47, 39)) >> 39)
#define PDPTE(addr) (((u64)addr & range_bitmask(38, 30)) >> 30)
#define PDE(addr) (((u64)addr & range_bitmask(29, 21)) >> 21)

#define PAGE_SHIFT (21)
#define PAGE_SIZE  (1<<PAGE_SHIFT)
#define PAGE_MASK  ((PAGE_NX) | (~(PAGE_SIZE-1)))

#define page_nr(addr) ((u64)(addr) >> PAGE_SHIFT)
#define page_addr(page) ((void *) ((u64)(page) << PAGE_SHIFT))

/*
* We use 2MB pages so our mapping looks like this:
*
*  pml4        4k table (512 PDPTE = 512G)
* [PML4E]---->[PDPTE]------->[PDE] (phyaddr) 4k table
*                    ----..(512 entries)     towards 2M pages
*             [PDPTE]------->[PDE] (phyaddr)
*                    ----..(512 entries)
*
* (Tables must be 4k-aligned)
*
*/

extern pg_desc_t *pml4;

#define pml4_desc(addr) (*(pg_desc_t *) (pml4 + \
    sizeof(pg_desc_t)*PML4E(addr)))
#define pdpt_desc(addr) (*(pg_desc_t *) (pg_desc_addr(pml4_desc(addr)) + \
    sizeof(pg_desc_t)*PDPTE(addr)))
#define page_desc(addr)  (*(pg_desc_t *) (pg_desc_addr(pdpt_desc(addr)) + \
    sizeof(pg_desc_t)*PDE(addr)))

#define virt_to_phys(addr) (pg_desc_addr(page_desc(addr)))

#endif /* ASM_FILE */
#endif /* _REGS_H */
