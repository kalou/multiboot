#include <mptable.h>
#include <kernel.h>
#include <asm.h>

u8 sig_csum(void *ptr, int len)
{
    u8 *c;
    u8 sum = 0;
    for (c = (u8 *) ptr; c < (u8 *) ptr + len; c++)
        sum += *c;
    return !sum;
}
#define CSUM_OK(p) (sig_csum(p, sizeof(*p)))

int is_apic(struct mp_floating *mp)
{
    if ((mp->signature == MP_FLOATING) && CSUM_OK(mp)) {
        if ((mp->version != 1) && (mp->version != 4))
            printk("We assume a strange APIC version here\n");
        printk("Found APIC v1.%d at %x\n", mp->version,
            mp->mp_config);
        if (mp->feature_byteN & (1 << 7))
            printk("ICMR here\n");
        return 1;
    }
    return 0;
}

void parse_apic(struct mp_config *mpc)
{
    char tmp[128] = { 0 };
    u8   *entryptr;
    int   i;
    struct mp_table_cpu *cpu;
    memcpy(tmp, mpc->oem, 8);
    memcpy(tmp+9, mpc->product, 12);

    entryptr = (u8 *) mpc + sizeof(*mpc);

    i = mpc->entries;
    printk("Lapic addr is %x, %d entries\n", mpc->lapic_addr, mpc->entries);

    while (i--)
    switch(((struct mp_table_entry *) entryptr)->type) {
        case ENTRY_CPU:
            cpu = (struct mp_table_cpu *) entryptr;
            printk("Found CPU %d\n", cpu->local_apic_id);
            entryptr += sizeof(struct mp_table_cpu);
            break;
        case ENTRY_BUS:
            printk("Found %s bus\n",
                ((struct mp_table_bus *) entryptr)->type_string);
            entryptr += sizeof(struct mp_table_bus);
            break;
        case ENTRY_IOAPIC:
            printk("Found an IO APIC at %x\n",
                ((struct mp_table_ioapic *) entryptr)->address);
            entryptr += sizeof(struct mp_table_ioapic);
            break;
        case ENTRY_LIRQM:
            /* cascade */
        case ENTRY_IRQM:
            entryptr += sizeof(struct mp_table_irqmap);
            break;
        default:
            i = 0;
            printk("Unknown mp table entry type %d\n",
                ((struct mp_table_entry *) entryptr)->type);
    }
}

/* Called from the boot processor - parses the FP config */
void mp_scan(void)
{
    int i;
    struct mp_floating *mp;
    start_end_t scan[] = {
        { 0x9fc00, 0x9fc00 + 1024 },
        { 0xf0000, 0xfffff },
        { 0, 0 }
    };


    for (i = 0; scan[i].start != 0; i++) {
        for (mp = (struct mp_floating *) scan[i].start;
            mp < (struct mp_floating *) scan[i].end; mp++) {
            if (is_apic(mp)) {
                parse_apic((struct mp_config *) (u64) mp->mp_config);
            }
        }
    }
}
