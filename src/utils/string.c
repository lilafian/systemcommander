/* SPDX-License-Identifier: GPL-3.0-or-later
 * Defines functions included in the C standard library for working with strings.
 * Copyright (C) 2026 lilaf */

#include <syscom/string.h>

size_t strlen(const char *s) {
        size_t len = 0;
        while (*s != '\0') {
                len++;
                s++;
        }
        return len;
}

char *stpcpy(char *restrict dst, char *restrict src) {
        char *p;
        p = mempcpy(dst, src, strlen(src));
        *p = '\0';
        return p;
}

char *strcpy(char *restrict dst, char *restrict src) {
        stpcpy(dst, src);
        return dst;
}

char *strcat(char *restrict dst, char *restrict src) {
        stpcpy(dst + strlen(dst), src);
        return dst;
}

char *strchrcat(char *dst, const char src) {
        char *ptr = dst;
        while (*ptr) ptr++;
        *ptr++ = src;
        *ptr = '\0';
        return dst;
}

char *itoa(uint64_t val, int base) {
        if (base > 16) base = 16;
        if (val == 0) return "0";

        static char buf[64] = {0};

        int i = 22;
        for (; val && i; --i, val /= base) {
                buf[i] = "0123456789abcdef"[val % base];
        }

        return &buf[i+1];
}

int strncmp(const char *s1, const char *s2, size_t n) {
        while (n && *s1 && ( *s1 == *s2 ) ) {
                ++s1;
                ++s2;
                --n;
        }
        
        if (n == 0) {
                return 0;
        } else {
                return ( *(unsigned char *)s1 - *(unsigned char *)s2 );
        }
}

int strcmp(const char* s1, const char* s2)
{
        while(*s1 && (*s1 == *s2))
        {
                s1++;
                s2++;
        }
        return *(const unsigned char*)s1 - *(const unsigned char*)s2;
}
