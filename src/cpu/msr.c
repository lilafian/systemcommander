/* SPDX-License-Identifier: GPL-3.0-or-later
 * Defines functions for reading/writing to MSRs (wrmsr/rdmsr).
 * Copyright (C) 2026 lilaf */

#include <syscom/msr.h>

void write_msr(uint32_t msr, uint64_t value) {
        uint32_t lo = (uint32_t)(value & 0xFFFFFFFF);
        uint32_t up = (uint32_t)(value >> 32);
        __asm__ volatile ("wrmsr" : : "c"(msr), "a"(lo), "d"(up));
}

uint64_t read_msr(uint32_t msr) {
        uint32_t lo;
        uint32_t up;
        __asm__ volatile ("rdmsr" : "=a"(lo), "=d"(up) : "c"(msr));
        return ((uint64_t)up << 32) | lo;
}

