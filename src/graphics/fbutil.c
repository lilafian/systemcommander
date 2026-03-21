#include <syscom/fbutil.h>
#include <syscom/stdmemory.h>

void fb_clear(struct limine_framebuffer *fb, uint32_t color) {
    uint32_t *pixels = (uint32_t *)fb->address;
    size_t pixel_count = (fb->pitch / sizeof(uint32_t)) * fb->height;

    for (size_t i = 0; i < pixel_count; i++) {
        pixels[i] = color;
    }
}
