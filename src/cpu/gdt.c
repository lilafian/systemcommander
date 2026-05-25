/* SPDX-License-Identifier: GPL-3.0-or-later
 * Defines the default_gdt structure.
 * Copyright (C) 2026 lilaf */

#include <syscom/gdt.h>

__attribute__((aligned(0x1000))) gdt default_gdt = {
    .null = {
        0, 0, 0, 0x00, 0x00, 0
    },
    .kernel_code = {
        0, 0, 0, 0x9a, 0xa0, 0
    },
    .kernel_data = {
        0, 0, 0, 0x92, 0xa0, 0
    },
    .user_null = {
        0, 0, 0, 0x00, 0x00, 0
    }, 
    .user_code = {
        0, 0, 0, 0xfa, 0xa0, 0
    },
    .user_data = {
        0, 0, 0, 0xf2, 0xa0, 0
    },
};

void gdt_install_tss(uint64_t base, uint32_t limit) {
        default_gdt.tss = (gdt_entry16){
                .limit0      = limit & 0xffff,
                .base0       = base & 0xffff,
                .base1       = (base >> 16) & 0xff,
                .access_byte = 0x89,
                .limit1_flags = (limit >> 16) & 0x0f,
                .base2       = (base >> 24) & 0xff,
                .base3       = (base >> 32) & 0xffffffff,
                ._reserved    = 0,
        };
}
