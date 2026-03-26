#pragma once

#include <stdint.h>
#include <limine.h>

void fb_clear(struct limine_framebuffer *fb, uint32_t color);
void fb_pixel(struct limine_framebuffer *fb, int x, int y, uint32_t color);
void fb_rect(struct limine_framebuffer *fb, uint32_t color, int x, int y, int width, int height);
