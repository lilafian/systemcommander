#include <syscom/heap.h>
#include <syscom/page_map.h>
#include <syscom/phys_allocator.h>
#include <syscom/log.h>

void *heap_start;
void *heap_end;
heap_segment_header *last_segment;

void heap_init(void *t_heap_start, size_t size) {
        uint64_t pos = (uint64_t)t_heap_start;
        for (size_t i = 0; i < size; i++) {
                uint64_t phys = (uint64_t)request_page();
                map_virtual_memory(kernel_pml4, pos, phys, PAGE_RW | PAGE_USER);
                pos += 0x1000;
        }

        size_t size_bytes = size * 0x1000;

        heap_start = t_heap_start;
        heap_end = (void*)((size_t)heap_start + size_bytes);
        heap_segment_header *start_segment = (heap_segment_header *)heap_start;
        start_segment->size = size_bytes - sizeof(heap_segment_header);
        start_segment->next = NULL;
        start_segment->last = NULL;
        start_segment->free = true;
        last_segment = start_segment;
}

void *malloc(size_t size) {
        if (size % 0x10 > 0) {
                size -= (size % 0x10);
                size += 0x10;
        }

        if (size == 0) return NULL;

        heap_segment_header *current = (heap_segment_header *)heap_start;
        while (true) {
                if (current->free) {
                        if (current->size > size) {
                                heapseg_split(current, size);
                                current->free = false;
                                return (void*)((uint64_t)current + sizeof(heap_segment_header));
                        }
                        if (current->size == size) {
                                current->free = false;
                                return (void*)((uint64_t)current + sizeof(heap_segment_header));
                        }
                }
                if (current->next == NULL) break;
                current = current->next;
        }

        expand_heap(size);
        return malloc(size);
}

void free(void *address) {
        heap_segment_header *segment = (heap_segment_header *)address - 1;
        segment->free = true;
        heapseg_combine_fwd(segment);
        heapseg_combine_bwd(segment);
}

void heapseg_combine_fwd(heap_segment_header *segment) {
        if (!segment->next || !segment->next->free) return;

        if (segment->next == last_segment) last_segment = segment;

        if (segment->next->next != NULL) segment->next->next->last = segment;

        segment->size += segment->next->size + sizeof(heap_segment_header);
        segment->next = segment->next->next;
}

void heapseg_combine_bwd(heap_segment_header *segment) {
        if (segment->last && segment->last->free) heapseg_combine_fwd(segment->last);
}

heap_segment_header *heapseg_split(heap_segment_header *segment, size_t first_size) {
        if (first_size < 0x10) return NULL;

        int64_t split_length = segment->size - first_size - sizeof(heap_segment_header);
        if (split_length < 0x10) return NULL;

        heap_segment_header *new = (heap_segment_header *)((size_t)segment + first_size + sizeof(heap_segment_header));
        if (segment->next) segment->next->last = new;
        new->next = segment->next;
        segment->next = new;
        new->last = segment;
        new->size = split_length;
        new->free = segment->free;
        segment->size = first_size;

        if (last_segment == segment) last_segment = new;
        return new;
}

void expand_heap(size_t amount) {
        if (amount % 0x1000 > 0) {
                amount -= amount % 0x1000;
                amount += 0x1000;
        }

        size_t page_count = amount / 0x1000;
        heap_segment_header *new = (heap_segment_header *)heap_end;

        for (size_t i = 0; i < page_count; i++) {
                map_virtual_memory(kernel_pml4, (uint64_t)heap_end, (uint64_t)request_page(), PAGE_RW | PAGE_USER);
                heap_end = (void *)((size_t)heap_end + 0x1000);
        }

        new->free = true;
        new->last = last_segment;
        last_segment->next = new;
        last_segment = new;
        new->next = NULL;
        new->size = amount - sizeof(heap_segment_header);
        heapseg_combine_bwd(new);
}
