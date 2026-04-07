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
        0, 0, 0, 0x9a, 0xa0, 0
    },
    .user_data = {
        0, 0, 0, 0x92, 0xa0, 0
    },
};
