/* SPDX-License-Identifier: GPL-3.0-or-later
 * Declares functions for reading/writing to MSRs (wrmsr/rdmsr).
 * Copyright (C) 2026 lilaf */

#pragma once
#include <stdint.h>

void write_msr(uint32_t msr, uint64_t value);
uint64_t read_msr(uint32_t msr);
