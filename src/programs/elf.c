/* SPDX-License-Identifier: GPL-3.0-or-later
 * Defines functions for loading and using ELF files.
 * Copyright (C) 2026 lilaf */

#include <syscom/elf.h>
#include <syscom/string.h>
#include <syscom/phys_allocator.h>
#include <syscom/page_map.h>
#include <syscom/log.h>
#include <syscom/ring3.h>

void elf_load(char *data, size_t data_size) {
        if (data_size < sizeof(elf_header)) {
                logf("[prog:elf_load] Data chunk smaller than ELF header\n");
        }

        if (strncmp(data, ELF_MAGIC, 4) != 0) {
                log("[prog:elf_load] Attempted to load a non-ELF data chunk. magic = ");
                for (int i = 0; i < 4; i++) {
                        logf("%c", data[i]);
                }
                log("\n");
                return;
        }

        elf_header *header = (elf_header *)data;

        for (uint64_t i = 0; i < header->phdr_entry_count; i++) {
                uint64_t offset = i * header->phdr_entry_size;
                elf_program_header *phdr = (elf_program_header *)((uint64_t)data + header->phdr_table_offset + offset);
                if (phdr->type != PT_LOAD) continue;

                void *phys = request_pages((phdr->size_memory + 0x1000 - 1) / 0x1000);
                for (uint64_t j = 0; j < (phdr->size_memory + 0x1000 - 1) / 0x1000; j++) {
                        uint64_t virtual_orig = phdr->virtual_base + j * 0x1000;
                        uint64_t physical_orig = (uint64_t)phys + j * 0x1000;
                        uint64_t virtual_aligned = PAGE_ALIGN(virtual_orig);
                        uint64_t physical_aligned = physical_orig - (virtual_orig - virtual_aligned);

                        map_virtual_memory(kernel_pml4, virtual_aligned, physical_aligned, PAGE_PRESENT | PAGE_RW | PAGE_USER);
                }
                memset((void *)phdr->virtual_base, 0, phdr->size_memory);
                memcpy((void *)phdr->virtual_base, (void *)((uint64_t)data + phdr->data_offset), phdr->size_file);
        }

        for (uint64_t i = 0; i < USER_STACK_SIZE + 1; i += 0x1000) {
                uint64_t phys = (uint64_t)request_page();
                map_virtual_memory(kernel_pml4, USER_STACK_TOP - USER_STACK_SIZE + i, phys, PAGE_PRESENT | PAGE_RW | PAGE_USER);
        }

        jump_usermode(header->entry, USER_STACK_TOP);
}
