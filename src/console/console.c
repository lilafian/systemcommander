/* SPDX-License-Identifier: GPL-3.0-or-later
 * Basic console functions.
 * Copyright (C) 2026 lilaf */

#include <syscom/console.h>
#include <syscom/stdmemory.h>
#include <syscom/string.h>
#include <syscom/fbutil.h>
#include <syscom/psf.h>

void console_init(console *con, struct limine_framebuffer *framebuffer, psf2_header_t *font, uint32_t fg_color, uint32_t bg_color) {
        con->framebuffer = framebuffer;
        con->font = font;
        con->fg_color = fg_color;
        con->bg_color = bg_color;
        con->length = 0;
        con->lines = 0;
        con->cursor_x = 0;
        con->cursor_y = 0;
        memset(con->buf, 0, CONSOLE_BUF_SIZE);
}

void console_write(console *con, const char *str) {
        size_t to_copy = strlen(str);

        if (con->length + to_copy + 1 > CONSOLE_BUF_SIZE) {
                size_t overflow = (con->length + to_copy + 1) - CONSOLE_BUF_SIZE;

                if (overflow > con->length) {
                        overflow = con->length;
                }

                memmove(con->buf, con->buf + overflow, con->length - overflow);
                con->length -= overflow;
        }

        memcpy(con->buf + con->length, str, to_copy);
        con->buf[con->length + to_copy] = '\0';

        int usable_height = con->framebuffer->height - (con->framebuffer->height % con->font->height);
        int max_lines = usable_height / con->font->height;
        for (int i = 0; i < to_copy; i++) {
                if (str[i] == '\n') con->lines++;
        }
        if (con->lines > max_lines) {
                memmove(
                                con->framebuffer->address,
                                con->framebuffer->address + con->font->height * con->framebuffer->pitch,
                                con->framebuffer->height * con->framebuffer->pitch - con->font->height * con->framebuffer->pitch
                        );
                fb_rect(
                                con->framebuffer,
                                con->bg_color,
                                0,
                                con->framebuffer->height - con->font->height,
                                con->framebuffer->width,
                                con->font->height
                        );
                con->cursor_y = usable_height - con->font->height;
                con->lines = max_lines;
        }

        con->cursor_x = psf2_draw_string(con->font, con->framebuffer, str, con->cursor_x, con->cursor_y, con->fg_color, con->bg_color);
        con->cursor_y = con->lines * con->font->height;
}

void console_clear(console *con) {
        memset(con->buf, 0, CONSOLE_BUF_SIZE);
        fb_clear(con->framebuffer, con->bg_color);
}
