/* SPDX-License-Identifier: GPL-3.0-or-later
 * Defines functions for drawing characters and strings with a PC Screen Font file.
 * Copyright (C) 2026 lilaf */

#include <syscom/psf.h>

void psf2_draw_char(psf2_header_t *font, struct limine_framebuffer *framebuffer, char c, int x, int y, uint32_t fg, uint32_t bg) {
        unsigned char *glyph = (unsigned char *)font + font->header_size +
                (c > 0 && (uint32_t)c < font->glyph_count ? c : 0) * font->glyph_size;

        int bytes_per_glyph_line = (font->width + 7) / 8;
        int row_offset = y * framebuffer->pitch;

        for (uint32_t ty = 0; ty < font->height; ty++) {
                unsigned char *current_byte = glyph + ty * bytes_per_glyph_line;
                uint8_t mask = 1 << 7;

                for (uint32_t tx = 0; tx < font->width; tx++) {
                        uint32_t *pixel = (uint32_t *)((uint8_t *)framebuffer->address + row_offset + (x + tx) * 4);
                        *pixel = (*current_byte & mask) ? fg : bg;

                        mask >>= 1;
                        if (mask == 0) {
                                mask = 1 << 7;
                                current_byte++;
                        }
                }

                row_offset += framebuffer->pitch;
        }
}

int psf2_draw_string(psf2_header_t *font, struct limine_framebuffer *framebuffer, char *str, int x, int y, uint32_t fg, uint32_t bg) {
        int nx = x;
        int ny = y;
        while (*str != '\0') {
                if (*str == '\n') {
                        ny += font->height;
                        nx = x;
                        str++;
                        continue;
                }
                psf2_draw_char(font, framebuffer, *str, nx, ny, fg, bg);
                nx += font->width;
                str++;
        }

        return nx;
}
