/* SPDX-License-Identifier: GPL-3.0-or-later
 * Defines functions for mapping virtual pages to physical ones.
 * Copyright (C) 2026 lilaf */

#include <stddef.h>
#include <syscom/page_map.h>
#include <syscom/phys_allocator.h>
#include <syscom/hhdm.h>
#include <syscom/log.h>
#include <syscom/stdmemory.h>

size_t paging_phys_alloc() {
        size_t phys = (size_t)request_page();
        void *virt = (void*)(phys + hhdm_offset);
        memset(virt, 0, 0x1000);
        return phys;
}

page_table *kernel_pml4;
void map_virtual_memory(page_table *pml4, uint64_t virtual_address, uint64_t physical_address, uint64_t flags) {
        if (virtual_address % 0x1000 != 0) {
                logf("[paging:map_virtual_memory] attempted to map non-page-aligned virtual address 0x%x to physical address 0x%x\n", virtual_address, physical_address);
        }

        uint32_t pml4_index = PML4_ENTRY(virtual_address);
        uint32_t pdp_index = PDPT_ENTRY(virtual_address);
        uint32_t pd_index = PD_ENTRY(virtual_address);
        uint32_t pt_index = PT_ENTRY(virtual_address);

        if (!(pml4->entries[pml4_index] & PAGE_PRESENT)) {
                size_t target = paging_phys_alloc();
                pml4->entries[pml4_index] = target | PAGE_PRESENT | PAGE_RW | PAGE_USER;
        }
        size_t* pdp = (size_t*)(PDE_ADDR(pml4->entries[pml4_index]) + hhdm_offset);

        if (!(pdp[pdp_index] & PAGE_PRESENT)) {
                size_t target = paging_phys_alloc();
                pdp[pdp_index] = target | PAGE_PRESENT | PAGE_RW | PAGE_USER;
        }
        size_t* pd = (size_t*)(PDE_ADDR(pdp[pdp_index]) + hhdm_offset);

        if (!(pd[pd_index] & PAGE_PRESENT)) {
                size_t target = paging_phys_alloc();
                pd[pd_index] = target | PAGE_PRESENT | PAGE_RW | PAGE_USER;
        }
        size_t* pt = (size_t*)(PDE_ADDR(pd[pd_index]) + hhdm_offset);

        if (!physical_address) {
                pt[pt_index] = 0;
        } else {
                pt[pt_index] = (PAGE_PHYS_ADDR(physical_address)) | PAGE_PRESENT | flags;
        }

        asm volatile("invlpg %0" :: "m"(virtual_address));
}
