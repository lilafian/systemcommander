/* SPDX-License-Identifier: GPL-3.0-or-later
 * Declares a structure and functions for the TSS.
 * Copyright (C) 2026 lilaf */

#pragma once
#include <stdint.h>

typedef struct tss_entry {
        uint32_t _reserved0;
        uint64_t rsp0;
        uint64_t rsp1;
        uint64_t rsp2;
        uint64_t _reserved1;
        uint64_t ist[7];
        uint64_t _reserved2;
        uint16_t _reserved3;
        uint16_t iopb_offset;
} __attribute__((packed)) tss_entry;

void tss_init(uint64_t kernel_stack);
void tss_set_rsp0(uint64_t stack);
extern void load_tss();
