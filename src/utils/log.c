#include <syscom/log.h>
#include <syscom/fmt.h>

console_t *log_console;

void set_log_console(console_t *console) {
        log_console = console;
}

void log(const char *msg) {
        console_write(log_console, msg);
}

void logf(const char *fmt, ...) {
        va_list args;
        va_start(args, fmt);

        char buffer[1024];
        buffer[0] = '\0';

        vasprintf(buffer, fmt, args);

        va_end(args);
        
        log(buffer);
}
