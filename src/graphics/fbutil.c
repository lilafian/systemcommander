#include <syscom/fbutil.h>
#include <syscom/stdmemory.h>

void fb_clear(struct limine_framebuffer *fb, uint32_t color) {
    uint32_t *pixels = (uint32_t *)fb->address;
    size_t pixel_count = (fb->pitch / sizeof(uint32_t)) * fb->height;

    for (size_t i = 0; i < pixel_count; i++) {
        pixels[i] = color;
    }
}

void fb_pixel(struct limine_framebuffer *fb, int x, int y, uint32_t color) {
        *(uint32_t*)((uint64_t)fb->address + x * 4 + y * fb->pitch) = color;
}

void fb_rect(struct limine_framebuffer *fb, uint32_t color, int x, int y, int width, int height) {
        for (int nx = x; nx < x + width; nx++) {
                for (int ny = y; ny < y + height; ny++) {
                        fb_pixel(fb, nx, ny, color);
                }
        }
}
