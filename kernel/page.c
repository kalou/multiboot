#include <kernel.h>
#include <page.h>

/* free page list */
static page_list_t *pages;

/* our PML4 for CR3 */
pg_desc_t *pml4;

static void delay(void) { u64 c; for (c = 0; c < (1UL<<32); c++) { } }

/* Init a page_list_t and mark pages used */
void init_page_list(page_list_t *p, u64 page)
{
    p->start = page;
    p->next = NULL;
    memset(p->bitmap, '\xff', sizeof(p->bitmap));
}

void init_page_subsys(void)
{
    pml4 = (pg_desc_t *) PML4_ADDR;
}

/* Called by memory/e820 setup to initialize page subsystem
 * when first page is found */
void init_page_alloc(u64 first_page)
{
    pages = page_addr(first_page);
    init_page_list(pages, 0);
}

void trace_page(u64 addr)
{
    printk("Tracing %x\n", addr);
    printk("pml4e is %x\n", pml4_desc(addr));
    printk("pdpte is %x\n", pdpt_desc(addr));
    printk("pde is %x\n", page_desc(addr));
    printk("Page address is %x\n", pg_desc_addr(page_desc(addr)));
}

int page_present(u64 page)
{
    void *addr = page_addr(page);

    return pml4_desc(addr) && pdpt_desc(addr) && \
        (page_desc(addr) & PAGE_PRESENT);
}

/* Find and return the page_list_t responsible for
 * this page number - or the preceding page to be
 * extended. Caller should check with page_in_range
 */
page_list_t *find_page_list(u64 nr)
{
    page_list_t *p = pages;

    while (!page_in_range(p, nr) && p) {
        if ((p->next) && (nr >= p->next->start))
            p = p->next;
        else
            break;
    }

    return p;
}

/* Extend page_list in the current hosting page. If
 * required, link to a new page */
page_list_t *extend_page_list(page_list_t *p, u64 page)
{
    if ((((u64)p+1) & PAGE_MASK) !=
        (((u64)p) & PAGE_MASK)) {
        panic("Should grab into self, fixme\n");
    }

    p->next = p+1;
    p = p->next;
    p->next = NULL;

    init_page_list(p, page);

    return p;
}

/* Add this page to our free list */
void add_page(u64 nr)
{
    page_list_t *p;
    p = find_page_list(nr);

    if (!page_in_range(p, nr)) {
        p = extend_page_list(p, nr);
    }

    /* Special case: do not mark our
    pagelist free */
    if (page_nr(pages) == nr)
        return;

    page_set_free(p, nr);
    if (!page_free(p, nr)) {
        printk("Failed to free page %d\n", nr);
        print_page_list(p);
        while(1) {} ;
    }
}

void *get_pages(int qty)
{
    page_list_t *p;
    u64 nr;

    BUG_ON(qty > 64);

    for (p = pages; p; p = p->next) {
        for (nr = p->start; page_in_range(p, nr); nr++) {
            if (pagerange_free(p, nr, qty) && (pagerange_set_used(p, nr, qty)))
                    return page_addr(nr);
        }
    }

    debug_pages();
    panic("get_pages(%d) failed\n", qty);
    return NULL;
}

void put_pages(void *page, int qty)
{
    u64 nr = page_nr(page);
    page_list_t *p = find_page_list(nr);

    if (!page_in_range(p, nr))
        return;

    while(!pagerange_set_free(p, nr, qty));
}

u64 count_free_pages(page_list_t *p)
{
    int c;
    u64 cnt = 0;

    for (c = 0; c < BITMAPS_PER_LIST; c++) {
        cnt += 64 - count_bits(p->bitmap[c]);
    }

    return cnt;
}

void print_page_list(page_list_t *p)
{
    int c;
    printk("pagelist %d:\n", p->start);
    for (c = 0; c < BITMAPS_PER_LIST; c += 8) {
        printk("%x %x %x %x\n%x %x %x %x\n",
                p->bitmap[c], p->bitmap[c+1], p->bitmap[c+2], p->bitmap[c+3],
                p->bitmap[c+4], p->bitmap[c+5], p->bitmap[c+6], p->bitmap[c+7]);
    }
    printk(" next-->%x\n", p->next);
}

void debug_pages(void)
{
    page_list_t *p;

    for (p = pages; p; p = p->next) {
        print_page_list(p);
    }
}

void report_pages(void)
{
    page_list_t *p;
    u64 free = 0;

    for (p = pages; p; p = p->next) {
        free += count_free_pages(p);
    }
    printk("%d free pages\n", free);
}

/*
* Pre-alloc 512GB worth of PDT (2MB page, 512 PDT) -
* Pre-alloc a 2MB page for 256TB PDPT and map the first
* PDPT entries to our existing PDT. Mapping of non present
* PML4 entries will alloc another batch of 512GB. Map PML4E
* to our newly created PDPT.
*/
void switch_pagetable(void)
{
    u64 phyaddr = 0UL;
    pg_desc_t *pt, *pte;

    /* BUILD PDT */
    pt = get_page();
    for (pte = pt; pte < pt + PAGE_SIZE/sizeof(pt); pte++) {
        *pte = phyaddr | PAGE_SIZE_SET | PAGE_PRESENT | \
            PAGE_WRITE;
        phyaddr += PAGE_SIZE;
    }

    /* Build PDPT */
    phyaddr = (u64) pt;
    pt = get_page();
    for (pte = pt; pte < pt + 512; pte++) {
        *pte = phyaddr | PAGE_PRESENT | PAGE_USER | \
            PAGE_WRITE;
        phyaddr += 4096;
    }

    /* Point PML4E to our newly created PDPT */
    phyaddr = (u64) pt;
    for (pte = pml4; pte < pml4 + 512; pte++) {
        *pte = phyaddr | PAGE_PRESENT | PAGE_USER | \
            PAGE_WRITE;
        phyaddr += 4096;
    }
}

void map_virt_to_phys(u64 virt, u64 phys, u32 flags)
{
    page_desc(virt) = (phys & PAGE_MASK) | flags | PAGE_SIZE_SET;
}

void map_range(u64 start, u64 end, u64 phys, u32 flags)
{
    start &= PAGE_MASK;
    phys &= PAGE_MASK;

    for (; start < end; start += PAGE_SIZE, phys += PAGE_SIZE) {
        map_virt_to_phys(start, phys, flags);
    }
}

void identity_map(u64 virt, u32 flags)
{
    map_virt_to_phys(virt, virt, flags);
}

void identity_map_range(u64 start, u64 end, u32 flags)
{
    map_range(start, end, start, flags);
}
