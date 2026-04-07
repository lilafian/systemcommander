/* SPDX-License-Identifier: GPL-3.0-or-later
 * Declares structures and functions for a bitmap type.
 * Copyright (C) 2026 lilaf */

#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct bitmap {
        size_t size;
        uint8_t *buffer;
} bitmap;

bool bm_get(bitmap *bmap, uint64_t index);
void bm_set(bitmap *bmap, uint64_t index, bool value);
