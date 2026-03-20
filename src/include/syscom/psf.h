#pragma once

#include <stdint.h>
#include <limine.h>

#define PSF2_MAGIC 0x844ab572

typedef struct psf2_header_t {
        uint32_t magic;
        uint32_t version;
        uint32_t header_size;
        uint32_t flags;
        uint32_t glyph_count;
        uint32_t glyph_size;
        uint32_t height;
        uint32_t width;
} psf2_header_t;

void psf2_draw_char(psf2_header_t *font, struct limine_framebuffer *framebuffer, char c, int x, int y, uint32_t fg, uint32_t bg);
void psf2_draw_string(psf2_header_t *font, struct limine_framebuffer *framebuffer, char *str, int x, int y, uint32_t fg, uint32_t bg);
