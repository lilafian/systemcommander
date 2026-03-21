#pragma once

#include <stddef.h>
#include <stdint.h>
#include <syscom/stdmemory.h>

size_t strlen(const char *s);
char *stpcpy(char *restrict dst, char *restrict src);
char *strcpy(char *restrict dst, char *restrict src);
char *strcat(char *restrict dst, char *restrict src);
char *strchrcat(char *restrict dst, const char src);
char *itoa(uint64_t val, int base);
