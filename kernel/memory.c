#include <kernel.h>
#include <memory.h>
#include <apic.h>
#include <multiboot.h>

#include <page.h>

/* kernel boundaries */
extern u64 _start;
extern u64 _end;

static e820_map_t e820_map[128];
static int e820_count;

void load_e820(multiboot_info_t *mbi)
{
    multiboot_memory_map_t *mmap;

#define MTYPESTR(x) (x==1 ? "available" : (x==2 ? "reserved" : \
    (x==3 ? "ACPI" : (x==4 ? "unusable" : "unknown"))))

    printk("%dKB lower memory\n", mbi->mem_lower);
    if (!(mbi->flags & MULTIBOOT_INFO_MEM_MAP)) {
        panic("Fix me, no memory map\n");
    }

    e820_count = 0;
    printk("Kernel %x->%x (page_nr %d)\n", &_start, &_end, page_nr(&_start));
    for (mmap = (multiboot_memory_map_t *) (u64) mbi->mmap_addr;
        mmap <= (multiboot_memory_map_t *) (u64) (mbi->mmap_addr + mbi->mmap_length);
        mmap = (multiboot_memory_map_t *) ((u64) mmap + mmap->size + sizeof(int))) {

        if (mmap->len > (1UL<<32))
            /* Strange issue with GRUB, will need to rewrite this
            ** to use e820 map anyway. Just skip it. */
            continue;

        printk("\t%x-%x: %s(%d) (%d%s)\n",
            mmap->addr, mmap->addr+mmap->len,
            MTYPESTR(mmap->type), mmap->type,
            mmap->len > (1024*1024) ? mmap->len / (1024*1024) : mmap->len/1024,
            mmap->len > (1024*1024) ? "MB" : "KB");

        e820_map[e820_count].start = mmap->addr;
        e820_map[e820_count].end = mmap->addr + mmap->len;
        e820_map[e820_count].type = mmap->type;
        e820_count++;

        if (e820_count > 128) {
            panic("Too many e820 entries, fixme\n");
        }
    }
}

int mem_overlap(u64 s1, u64 e1, u64 s2, u64 e2)
{
    /* Check if we have a common memory region */
    if (s1 < s2)
        return (e1 > s2);
    else
        return (s1 < e2);
}

/* Check if e820 unavail memory between start and end */
int e820_ok(u64 start, u64 end)
{
    int e;

    for (e = 0; e < e820_count; e++) {
            /* Unavail overlap */
            if ((e820_map[e].type != 1) &&
                mem_overlap(start, end, e820_map[e].start, e820_map[e].end))
                return 0;

            if ((e820_map[e].type == 1) && start >= e820_map[e].start &&
                end <= e820_map[e].end) {
                return 1;
            }
    }

    /* Not in e820: unavail */
    return 0;
}

int kernel_ok(u64 start, u64 end)
{
   return !(mem_overlap(start, end, (u64) &_start, (u64) &_end));
}

int page_ok(u64 page)
{
    return kernel_ok((u64)page_addr(page), (u64)page_addr(page)+PAGE_SIZE) &&
        e820_ok((u64)page_addr(page), (u64)page_addr(page)+PAGE_SIZE);
}

/* Early find a free page using e820 */
u64 find_first_free_page(void)
{
    u64 i = 0;
    printk("Looking for an usable page for initial setup\n");

    for (i = 0; i < 512; i++) {
        if (!page_ok(i) || !page_present(i))
            continue;

        printk("Found page %d\n", i);
        return i;
    }

    panic("Cannot find first free page\n");
    return -1;
}

void setup_pagelist(void)
{
    int e, page;

    for (e = 0; e < e820_count; e++) {
        if (e820_map[e].type != 1)
            continue;

        for (page = page_nr(e820_map[e].start);
                page < page_nr(e820_map[e].end);
                page++) {
            if (page_ok(page))
                add_page(page);
        }
    }

    report_pages();
}

void init_memory(multiboot_info_t *mbi)
{
    u64 first_page;

    load_e820(mbi);
    init_page_subsys();

    first_page = find_first_free_page();
    init_page_alloc(first_page);
    setup_pagelist();

    switch_pagetable();
}
