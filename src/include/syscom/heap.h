#pragma once
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

typedef struct heap_segment_header {
        size_t size;
        struct heap_segment_header *next;
        struct heap_segment_header *last;
        bool free;
} heap_segment_header;

void heap_init(void *heap_start, size_t size);

void *malloc(size_t size);
void free(void *address);

void heapseg_combine_fwd(heap_segment_header *segment);
void heapseg_combine_bwd(heap_segment_header *segment);
heap_segment_header *heapseg_split(heap_segment_header *segment, size_t first_size);

void expand_heap(size_t amount);
