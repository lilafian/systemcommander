#pragma once

#include <limine.h>
#include <syscom/psf.h>

#define CONSOLE_BUF_SIZE 4096

typedef struct console_t {
        struct limine_framebuffer *framebuffer;
        psf2_header_t *font;
        uint32_t fg_color;
        uint32_t bg_color;
        char buf[CONSOLE_BUF_SIZE];
} console_t;

void console_init(console_t *con, struct limine_framebuffer *framebuffer, psf2_header_t *font, uint32_t fg_color, uint32_t bg_color);
void console_write(console_t *con, const char *str);
void console_clear(console_t *con);
