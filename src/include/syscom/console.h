#pragma once

#include <limine.h>
#include <syscom/psf.h>

#define CONSOLE_BUF_SIZE 4096

typedef struct console {
        struct limine_framebuffer *framebuffer;
        psf2_header_t *font;
        uint32_t fg_color;
        uint32_t bg_color;
        char buf[CONSOLE_BUF_SIZE];
        int length;
        int lines;
        int cursor_x;
        int cursor_y;
} console;

void console_init(console *con, struct limine_framebuffer *framebuffer, psf2_header_t *font, uint32_t fg_color, uint32_t bg_color);
void console_write(console *con, const char *str);
void console_clear(console *con);
