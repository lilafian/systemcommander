/* SPDX-License-Identifier: GPL-3.0-or-later
 * Defines functions for logging to the kernel log.
 * Copyright (C) 2026 lilaf */

#include <syscom/log.h>
#include <syscom/fmt.h>
#include <syscom/serial.h>

console *log_console;
uint8_t log_mode;

void set_log_mode(uint8_t mode) {
        log_mode = mode;
}

void set_log_console(console *console) {
        log_console = console;
}

void log(const char *msg) {
        if (log_mode == LOG_MODE_FRAMEBUFFER_SERIAL) {
                console_write(log_console, msg);
        }
        while (*msg != '\0') {
                serial_send(COM1, *msg);
                msg++;
        }
}

void logf(const char *fmt, ...) {
        va_list args;
        va_start(args, fmt);

        char buffer[1024];
        buffer[0] = '\0';

        vasprintf(buffer, fmt, args);

        va_end(args);
        
        log(buffer);
}
