/* SPDX-License-Identifier: GPL-3.0-or-later
 * Functions for modifying the IDT.
 * Copyright (C) 2026 lilaf */

#include <syscom/idt.h>

void idtdesce_set_offset(idt_descriptor_entry* entry, uint64_t offset) {
    entry->offset0 = (uint16_t)(offset & 0xffff);
    entry->offset1 = (uint16_t)((offset >> 16) & 0xffff);
    entry->offset2 = (uint32_t)((offset >> 32) & 0xffffffff);
}

uint64_t idtdesce_get_offset(idt_descriptor_entry* entry) {
    uint64_t offset = 0;
    offset |= (uint64_t)entry->offset0;
    offset |= (uint64_t)entry->offset1 << 16;
    offset |= (uint64_t)entry->offset2 << 32;
    return offset;
}
