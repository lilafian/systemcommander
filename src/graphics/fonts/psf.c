#include <syscom/psf.h>

void psf2_draw_char(psf2_header_t *font, struct limine_framebuffer *framebuffer, char c, int x, int y, uint32_t fg, uint32_t bg) {
        unsigned char *glyph = (unsigned char *)font + font->header_size + (c > 0 && c < font->glyph_count ? c : 0) * font->glyph_size;

        int offset = y * (framebuffer->width * framebuffer->bpp) + (x * sizeof(uint32_t));
        uint32_t bytes_per_glyph_line = (font->width + 7) / 8;

        int tx;
        int ty;
        int line;
        for (ty = 0; ty < font->height; ty++) {
                line = offset;
                unsigned char * current_byte = glyph + (bytes_per_glyph_line * ty);
                uint8_t mask = 1 << 7;
                for (tx = 0; tx < font->width; tx++) {
                        *((uint32_t *)(framebuffer->address + line)) = (*current_byte & mask) ? fg : bg;
                        mask >>= 1;
                        if (mask == 0) {
                                mask = 1 << 7;
                                current_byte++;
                        }

                        line += sizeof(uint32_t);
                }

                offset += framebuffer->width * (framebuffer->bpp / 8); // How else to get scanline? Or maybe i cant read
        }
}

void psf2_draw_string(psf2_header_t *font, struct limine_framebuffer *framebuffer, char *str, int x, int y, uint32_t fg, uint32_t bg) {
        int nx = x;
        while (*str != '\0') {
                psf2_draw_char(font, framebuffer, *str, nx, y, fg, bg);
                nx += font->width;
                str++;
        }
}
