/* SPDX-License-Identifier: GPL-3.0-or-later
 * Declares a function for getting the amount of system memory.
 * Copyright (C) 2026 lilaf */

#pragma once

#include <stdint.h>
#include <limine.h>

uint64_t get_memory_size(struct limine_memmap_response *map);
