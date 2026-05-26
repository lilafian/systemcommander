/* SPDX-License-Identifier: GPL-3.0-or-later
 * Declares structures and functions for using syscalls.
 * Copyright (C) 2026 lilaf */

#pragma once
#include <stdint.h>

uint64_t syscall_handler(uint64_t num, uint64_t rdi, uint64_t rsi, uint64_t rdx, uint64_t r10, uint64_t r8, uint64_t r9);
void init_syscalls();
