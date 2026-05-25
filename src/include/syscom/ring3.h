/* SPDX-License-Identifier: GPL-3.0-or-later
 * Declares a function for jumping to usermode.
 * Copyright (C) 2026 lilaf */

#pragma once
#include <stdint.h>

extern void jump_usermode(uint64_t entry, uint64_t user_stack_top);
