/* SPDX-License-Identifier: GPL-3.0-or-later
 * Declares functions for loading and using the GDT.
 * Copyright (C) 2026 lilaf */

#pragma once

#include <stdint.h>

typedef struct gdt_descriptor {
        uint16_t size;
        uint64_t offset;
}__attribute__((packed)) gdt_descriptor;

typedef struct gdt_entry {
        uint16_t limit0;
        uint16_t base0;
        uint8_t base1;
        uint8_t access_byte;
        uint8_t limit1_flags;
        uint8_t base2;
}__attribute__((packed)) gdt_entry;

typedef struct gdt {
        gdt_entry null;
        gdt_entry kernel_code;
        gdt_entry kernel_data;
        gdt_entry user_null;
        gdt_entry user_code;
        gdt_entry user_data;
}__attribute__((packed)) __attribute__((aligned(0x1000))) gdt;

extern gdt default_gdt;
extern void load_gdt(gdt_descriptor *gdt_desc);
