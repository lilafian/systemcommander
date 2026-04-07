/* SPDX-License-Identifier: GPL-3.0-or-later
 * Defines a function for getting the memory size.
 * Copyright (C) 2026 lilaf */


#include <syscom/memory.h>

uint64_t get_memory_size(struct limine_memmap_response *map) {
        static uint64_t memory_size = 0;
        if (memory_size != 0) return memory_size;
        
        for (uint64_t i = 0; i < map->entry_count; i++) {
                struct limine_memmap_entry *entry = map->entries[i];
                if (entry->type != LIMINE_MEMMAP_RESERVED) {
                        memory_size += entry->length;
                }
        }

        return memory_size;
}
