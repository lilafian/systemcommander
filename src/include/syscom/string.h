/* SPDX-License-Identifier: GPL-3.0-or-later
 * Declares functions included in the C standard library for working with strings.
 * Copyright (C) 2026 lilaf */

#pragma once

#include <stddef.h>
#include <stdint.h>
#include <syscom/stdmemory.h>

typedef uint16_t char16;

size_t strlen(const char *s);
char *stpcpy(char *restrict dst, char *restrict src);
char *strcpy(char *restrict dst, char *restrict src);
char *strcat(char *restrict dst, char *restrict src);
char *strchrcat(char *restrict dst, const char src);
char *itoa(uint64_t val, int base);
int strncmp(const char *s, const char *t, size_t n);
