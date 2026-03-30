#pragma once

#include <stdint.h>

#define PAGE_PRESENT   (1ULL << 0)
#define PAGE_RW        (1ULL << 1)
#define PAGE_USER      (1ULL << 2)
#define PAGE_PWT       (1ULL << 3)
#define PAGE_PCD       (1ULL << 4)
#define PAGE_ACCESSED  (1ULL << 5)
#define PAGE_DIRTY     (1ULL << 6)
#define PAGE_PS        (1ULL << 7)
#define PAGE_GLOBAL    (1ULL << 8)
#define PAGE_NX        (1ULL << 63)
#define PAGE_ADDR_MASK 0x000FFFFFFFFFF000ULL

#define PML4_ENTRY(a) (((a) >> 39) & 0x1ff)
#define PDPT_ENTRY(a) (((a) >> 30) & 0x1ff)
#define PD_ENTRY(a) (((a) >> 21) & 0x1ff)
#define PT_ENTRY(a) (((a) >> 12) & 0x1ff)

#define PDE_ADDR(a) ((a) & PAGE_ADDR_MASK)
#define PDE_FLAGS(a) ((a) & ~PAGE_ADDR_MASK)

#define PAGE_PHYS_ADDR(a) ((a) & ~0xFFF)

#define PAGE_ALIGN(a) ((a) - (a % 0x1000))

typedef uint64_t page_entry;

typedef struct page_table {
    page_entry entries[512];
} __attribute__((aligned(0x1000))) page_table;

extern page_table *kernel_pml4;

void map_virtual_memory(page_table *pml4, uint64_t virtual_address, uint64_t physical_address, uint64_t flags);
