#pragma once

#include <stdarg.h>

void sprintf(char *str, const char *fmt, ...);
void vasprintf(char *str, const char *fmt, va_list args);
