/* SPDX-License-Identifier: GPL-3.0-or-later
 * Declares functions included in the C standard library for managing memory contents.
 * Copyright (C) 2026 lilaf */

#pragma once

#include <stddef.h>

void *memcpy(void *restrict dest, const void *restrict src, size_t n);
void *mempcpy(void *restrict dest, const void *restrict src, size_t n);
void *memset(void *s, int c, size_t n);
void *memmove(void *dest, const void *src, size_t n);
int memcmp(const void *s1, const void *s2, size_t n);
