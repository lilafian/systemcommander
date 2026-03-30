#include <syscom/panic.h>
#include <syscom/log.h>

void panic(const char *msg) {
        logf("<FATAL> <PANIC> %s\n", msg);
        for (;;) {
                asm volatile ("hlt");
        }
}
