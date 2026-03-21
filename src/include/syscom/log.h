#pragma once

#include <stdarg.h>
#include <syscom/console.h>

void set_log_console(console_t *console);
void log(const char *msg);
void logf(const char *fmt, ...);
