#ifndef __IDT_H__
#define __IDT_H__

struct idt_dscrpt
{
    uint16_t offset_0_15;
    uint16_t seg_selector;
    uint8_t  zero;
    uint8_t  type;
    uint16_t offset_16_31;
    uint32_t offset_32_63;
    uint32_t reserved;
} __attribute__((packed));

#define IDT_SIZE 33

struct idt_dscrpt idt[IDT_SIZE];

static inline void initIDT()
{
    struct idt_ptr my_idt_ptr;

    my_idt_ptr.base = (uint64_t) idt;
    my_idt_ptr.size = IDT_SIZE * sizeof(struct idt_dscrpt) - 1;

    set_idt(&my_idt_ptr);
}

#endif /* __IDT_H__ */