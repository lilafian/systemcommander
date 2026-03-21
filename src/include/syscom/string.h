#pragma once

#include <stddef.h>
#include <syscom/stdmemory.h>

size_t strlen(const char *s);
char *stpcpy(char *restrict dst, char *restrict src);
char *strcpy(char *restrict dst, char *restrict src);
char *strcat(char *restrict dst, char *restrict src);
