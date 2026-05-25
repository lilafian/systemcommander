/* SPDX-License-Identifier: GPL-3.0-or-later
 * Defines functions for the TSS.
 * Copyright (C) 2026 lilaf */

#include <syscom/tss.h>
#include <syscom/gdt.h>

static tss_entry tss;

void tss_init(uint64_t kernel_stack) {
        tss = (tss_entry){0};
        tss.rsp0 = kernel_stack;
        tss.iopb_offset = sizeof(tss_entry);
        
        gdt_install_tss((uint64_t)&tss, sizeof(tss_entry) - 1);
}

void tss_set_rsp0(uint64_t stack) {
        tss.rsp0 = stack;
}
