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
