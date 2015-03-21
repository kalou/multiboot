#ifndef _MPTABLE_H
#define _MPTABLE_H

#include <types.h>

struct mp_floating {
    u32  signature; /* MP_FLOATING */
    u32  mp_config;
    u8   length;
    u8   version;
    u8   cksum;
    u8   feature_byte1;
    u32  feature_byteN;
} __attribute__((packed));
#define MP_FLOATING (0x5f504d5f) /* _MP_ */

struct mp_config {
    u32  signature; /* MP_CONFIG */
    u16  length;
    u8   rev;
    u8   csum;
    char oem[8];
    char product[12];
    u32  oem_config;
    u16  oem_config_size;
    u16  entries;
    u32  lapic_addr;
    u16  ext_length;
    u8   ext_csum;
    u8   rsvd;
} __attribute__((packed));
#define MP_CONFIG   (0x504d4350) */ PCMP */

/* MP config table entries */

struct mp_table_entry {
    u8 type;
    u8 align1;
    u16 align2;
} __attribute__((packed));

struct mp_table_cpu {
    u8  type; /* ENTRY_CPU */
    u8  local_apic_id;
    u8  local_apic_version;
    u8  cpu_en: 1, cpu_bp: 1, cpu_rsvd: 6;
    u32 cpu_signature;
    u32 feature_flags;
    u64 rsvd;
} __attribute__((packed));
#define ENTRY_CPU 0

struct mp_table_bus {
    u8  type; /* ENTRY_BUS */
    u8  bus_id;
    char type_string[6];
} __attribute__((packed));
#define ENTRY_BUS 1

struct mp_table_ioapic {
    u8  type; /* ENTRY_IOAPIC */
    u8  ioapic_id; 
    u8  version;
    u8  flags: 3, rsvd: 5;
    u32 address;
} __attribute__((packed));
#define ENTRY_IOAPIC 2

struct mp_table_irqmap {
    u8  type; /* ENTRY_IRQM */
    u8  irq_type;
    u16 po: 2, el: 2, rsvd: 12;
    u8  src_bus_id;
    u8  src_bus_irq;
    u8  dst_apic_id;
    u8  dst_apic_INTIN;
} __attribute__((packed));
#define ENTRY_IRQM 3  /* irq map */
#define ENTRY_LIRQM 4 /* local irq map */

void mp_scan(void);

#endif /* _APIC_H */
