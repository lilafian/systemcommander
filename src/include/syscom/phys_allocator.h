/* SPDX-License-Identifier: GPL-3.0-or-later
 * Declares functions for using the physical memory allocator.
 * Copyright (C) 2026 lilaf */

#pragma once

#include <stdint.h>
#include <limine.h>
#include <syscom/memory.h>

void read_memory_map(struct limine_memmap_response *map);
void free_page(void *addr);
void lock_page(void *addr);
void free_pages(void *addr, uint64_t count);
void lock_pages(void *addr, uint64_t count);
void *request_page();
void *request_pages(int count);

uint64_t get_free_memory();
uint64_t get_used_memory();
uint64_t get_reserved_memory();
