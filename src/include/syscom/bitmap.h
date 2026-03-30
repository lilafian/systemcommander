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
