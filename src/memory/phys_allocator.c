/* SPDX-License-Identifier: GPL-3.0-or-later
 * Defines functions for using the physical memory allocator.
 * Copyright (C) 2026 lilaf */

#include <syscom/phys_allocator.h>
#include <syscom/bitmap.h>
#include <syscom/stdmemory.h>
#include <syscom/hhdm.h>

bitmap page_bitmap;
uint64_t free_memory;
uint64_t reserved_memory;
uint64_t used_memory;
bool initialized = false;

void free_page(void *addr) {
        uint64_t index = (uint64_t)addr / 4096;
        if (bm_get(&page_bitmap, index) == false) return;
        bm_set(&page_bitmap, index, false);
        free_memory += 4096;
        used_memory -= 4096;
}

void lock_page(void *addr) {
        uint64_t index = (uint64_t)addr / 4096;
        if (bm_get(&page_bitmap, index) == true) return;
        bm_set(&page_bitmap, index, true);
        free_memory -= 4096;
        used_memory += 4096;
}

void reserve_page(void *addr) {
        uint64_t index = (uint64_t)addr / 4096;
        if (bm_get(&page_bitmap, index) == true) return;
        bm_set(&page_bitmap, index, true);
        free_memory -= 4096;
        reserved_memory += 4096;
}

void release_page(void *addr) {
        uint64_t index = (uint64_t)addr / 4096;
        if (bm_get(&page_bitmap, index) == false) return;
        bm_set(&page_bitmap, index, false);
        free_memory += 4096;
        reserved_memory -= 4096;
}

void free_pages(void *addr, uint64_t count) {
        for (uint64_t i = 0; i < count; i++) {
                free_page(addr + (i * 4096));
        }
}

void lock_pages(void *addr, uint64_t count) {
        for (uint64_t i = 0; i < count; i++) {
                lock_page(addr + (i * 4096));
        }
}

void reserve_pages(void *addr, uint64_t count) {
        for (uint64_t i = 0; i < count; i++) {
                reserve_page(addr + (i * 4096));
        }
}

void release_pages(void *addr, uint64_t count) {
        for (uint64_t i = 0; i < count; i++) {
                release_page(addr + (i * 4096));
        }
}

// TODO: OPTIMIZE!!!
void *request_page() {
        for (uint64_t index = 0; index < page_bitmap.size * 8; index++) {
                if (bm_get(&page_bitmap, index) == true) continue;
                lock_page((void *)(index * 4096));
                return (void *)(index * 4096);
        }

        // TODO: swap partition/file

        return NULL;
}

void read_memory_map(struct limine_memmap_response *map) {
        if (initialized) return;
        initialized = true;

        void* largest = NULL;
        size_t largest_size = 0;

        for (uint64_t i = 0; i < map->entry_count; i++) {
                struct limine_memmap_entry *entry = map->entries[i];
                if (entry->type == LIMINE_MEMMAP_USABLE) {
                        if (entry->length > largest_size) {
                                largest = (void*)entry->base;
                                largest_size = entry->length;
                        }
                }
        }

        uint64_t memory_size = get_memory_size(map);
        free_memory = memory_size;
        uint64_t bitmap_size = memory_size / 4096 / 8 + 1;

        page_bitmap.size = bitmap_size;
        page_bitmap.buffer = (uint8_t *)largest + hhdm_offset;
        memset(page_bitmap.buffer, 0, page_bitmap.size);

        lock_pages((void *)largest, (bitmap_size + 4095) / 4096);

        for (uint64_t i = 0; i < map->entry_count; i++) {
                struct limine_memmap_entry *entry = map->entries[i];
                if (entry->type != LIMINE_MEMMAP_USABLE && entry->type != LIMINE_MEMMAP_RESERVED) {
                        reserve_pages((void*)entry->base, (entry->length + 4095) / 4096);
                }
        }
        
        lock_page((void *)0x0);
}

uint64_t get_free_memory() {
        return free_memory;
}

uint64_t get_used_memory() {
        return used_memory;
}

uint64_t get_reserved_memory() {
        return reserved_memory;
}
