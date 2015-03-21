#ifndef _PAGE_H
#define _PAGE_H

#include <kernel.h>
#include <asm.h>
#include <atomic.h>
#include <regs.h>
#include <multiboot.h>

#pragma GCC diagnostic ignored "-Wunused-function"

#define BITMAPS_PER_LIST 8

typedef struct _page_list {
    u64 start; /* nr */
    u64 bitmap[BITMAPS_PER_LIST];
    struct _page_list *next;
} page_list_t;

#define PAGES_PER_LIST BITMAPS_PER_LIST*64
#define PAGE_LIST_PER_PAGE (PAGE_SIZE/sizeof(page_list_t))

#define _page_nr(p, nr) ((nr) - p->start)
#define _bitmap_idx(p, nr) (_page_nr(p, nr) / 64)
#define _bitmap_bit(p, nr) (_page_nr(p, nr) % 64)

#define page_set_free(p, nr) ( \
    atomic_clear_bit(&p->bitmap[_bitmap_idx(p, nr)], _bitmap_bit(p, nr)))
#define pagerange_set_free(p, nr, qty) ( \
    atomic_clear_range(&p->bitmap[_bitmap_idx(p, nr)], (_bitmap_bit(p, nr) + qty - 1), \
        _bitmap_bit(p, nr)))

#define page_set_used(p, nr) ( \
    atomic_set_bit(&p->bitmap[_bitmap_idx(p, nr)], _bitmap_bit(p, nr)))
#define pagerange_set_used(p, nr, qty) ( \
    atomic_set_range(&p->bitmap[_bitmap_idx(p, nr)], (_bitmap_bit(p, nr) + qty - 1), \
        _bitmap_bit(p, nr)))

#define page_free(p, nr) bit_clear(p->bitmap[_bitmap_idx(p, nr)], \
    _bitmap_bit(p, nr))

#define range_ok(p, nr, qty) (_bitmap_bit(p, nr) <= _bitmap_bit(p, nr+qty-1))
#define _pagerange_free(p, nr, qty) range_clear(p->bitmap[_bitmap_idx(p, nr)], \
    (_bitmap_bit(p, nr) + qty - 1), _bitmap_bit(p, nr))

#define pagerange_free(p, nr, qty) \
    (range_ok(p, nr, qty) &&  _pagerange_free(p, nr, qty))

#define page_in_range(p, page) ((page >= p->start) && \
    (page < p->start + PAGES_PER_LIST))


void init_page_subsys(void);
void init_page_alloc(u64 first_page);
void trace_page(u64 addr);
int page_present(u64 nr);
void init_memory(multiboot_info_t *mbi);
void map_virt_to_phys(u64 virt, u64 phys, u32 flags);
void map_range(u64 start, u64 end, u64 phys, u32 flags);
void identity_map(u64 virt, u32 flags);
void identity_map_range(u64 start, u64 end, u32 flags);

void print_page_list(page_list_t *p);

void add_page(u64 page);

void *get_pages(int nr);
static void *get_page(void) { return get_pages(1); }

void put_pages(void *p, int nr);
static void put_page(void *p) { return put_pages(p, 1); }

void report_pages(void);
void debug_pages(void);

void switch_pagetable(void);


#endif /* _PAGE_H */
