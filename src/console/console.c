#include <syscom/console.h>
#include <syscom/stdmemory.h>
#include <syscom/string.h>
#include <syscom/fbutil.h>
#include <syscom/psf.h>

void console_init(console_t *con, struct limine_framebuffer *framebuffer, psf2_header_t *font, uint32_t fg_color, uint32_t bg_color) {
        con->framebuffer = framebuffer;
        con->font = font;
        con->fg_color = fg_color;
        con->bg_color = bg_color;
        memset(con->buf, 0, CONSOLE_BUF_SIZE);
}

void console_draw(console_t *con) {
        fb_clear(con->framebuffer, con->bg_color);
        psf2_draw_string(con->font, con->framebuffer, con->buf, 0, 0, con->fg_color, con->bg_color);
}

void console_write(console_t *con, const char *str) {
        size_t len = strlen(con->buf);
        size_t to_copy = strlen(str);

        if (len + to_copy >= CONSOLE_BUF_SIZE) {
                to_copy = CONSOLE_BUF_SIZE - len - 1; // leave space for null
        }

        memcpy(con->buf + len, str, to_copy);
        con->buf[len + to_copy] = '\0';

        console_draw(con);
}

void console_clear(console_t *con) {
        memset(con->buf, 0, CONSOLE_BUF_SIZE);
        console_draw(con);
}
