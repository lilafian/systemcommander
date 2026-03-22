#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct bitmap_t {
        size_t size;
        uint8_t *buffer;
} bitmap_t;

bool bm_get(bitmap_t *bitmap, uint64_t index);
void bm_set(bitmap_t *bitmap, uint64_t index, bool value);
