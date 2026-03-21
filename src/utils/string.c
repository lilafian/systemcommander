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
