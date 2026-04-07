/* SPDX-License-Identifier: GPL-3.0-or-later
 * Defines functions for using a bitmap.
 * Copyright (C) 2026 lilaf */

#include <syscom/bitmap.h>

bool bm_get(bitmap *bmap, uint64_t index) {
        uint64_t byte = index / 8;
        uint64_t bit = index % 8;
        uint8_t indexer = 0b10000000 >> bit;

        if ((bmap->buffer[byte] & indexer) > 0) return true;
        return false;
}

void bm_set(bitmap *bmap, uint64_t index, bool value) {
        uint64_t byte = index / 8;
        uint64_t bit = index % 8;
        uint8_t indexer = 0b10000000 >> bit;
        bmap->buffer[byte] &= ~indexer;
        if (value) {
                bmap->buffer[byte] |= indexer;
        }
}
