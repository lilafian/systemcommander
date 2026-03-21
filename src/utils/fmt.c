#include <stdint.h>
#include <syscom/fmt.h>
#include <syscom/string.h>

void sprintf(char* str, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vasprintf(str, fmt, args);
    va_end(args);
}

void vasprintf(char* str, const char* fmt, va_list args) {
    char buffer[1024];
    buffer[0] = '\0';

    while (*fmt) {
        if (*fmt == '%') {
            fmt++;
            switch(*fmt) {
                case 's': { // string
                    char* string = va_arg(args, char*);
                    strcat(buffer, string);
                    break;
                }
                case 'c': { // character
                    char character = (char)va_arg(args, int);
                    strchrcat(buffer, character);
                    break;
                }
                case 'd': { // decimal uint
                    uint64_t value = va_arg(args, uint64_t);
                    strcat(buffer, itoa(value, 10));
                    break;
                }
                case 'x': {// hex uint
                    uint64_t value = va_arg(args, uint64_t);
                    strcat(buffer, itoa(value, 16));
                    break;
                }
                case 'p': {
                    void* value = va_arg(args, void*);
                    strcat(buffer, itoa((uint64_t)value, 16));
                    break;
                }
                case '%': {
                    strcat(buffer, "%");
                    break;
                }
                default: {
                    strcat(buffer, "%?");
                    break;
                }
            }
        } else {
            char temp[2] = {*fmt, '\0'};
            strcat(buffer, temp);
        }
        fmt++;
    }

    strcpy(str, buffer);
}
