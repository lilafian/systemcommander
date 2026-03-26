#pragma once

#include <stdarg.h>
#include <stdint.h>
#include <syscom/console.h>

#define LOG_MODE_FRAMEBUFFER_SERIAL 0
#define LOG_MODE_SERIAL_ONLY 1

void set_log_console(console_t *console);
void set_log_mode(uint8_t mode);
void log(const char *msg);
void logf(const char *fmt, ...);
