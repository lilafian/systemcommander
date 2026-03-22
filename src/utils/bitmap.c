#include <syscom/bitmap.h>

bool bm_get(bitmap_t *bitmap, uint64_t index) {
        uint64_t byte = index / 8;
        uint64_t bit = index % 8;
        uint8_t indexer = 0b10000000 >> bit;

        if ((bitmap->buffer[byte] & indexer) > 0) return true;
        return false;
}

void bm_set(bitmap_t *bitmap, uint64_t index, bool value) {
        uint64_t byte = index / 8;
        uint64_t bit = index % 8;
        uint8_t indexer = 0b10000000 >> bit;
        bitmap->buffer[byte] &= ~indexer;
        if (value) {
                bitmap->buffer[byte] |= indexer;
        }
}
