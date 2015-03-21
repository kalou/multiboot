#ifndef _MEMORY_H
#define _MEMORY_H

#include <asm.h>
#include <regs.h>
#include <multiboot.h>

typedef struct _e820_map {
    u64 start;
    u64 end;
    int type;
} e820_map_t;

void init_memory(multiboot_info_t *mbi);

#endif
