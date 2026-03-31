#include <syscom/panic.h>
#include <syscom/log.h>
#include <syscom/fmt.h>

void panic(const char *msg) {
        logf("<FATAL> <PANIC> %s\n", msg);
        for (;;) {
                asm volatile ("hlt");
        }
}

void panicf(const char *fmt, ...) {
        va_list args;
        va_start(args, fmt);
        char buf[1024];
        vasprintf(buf, fmt, args);
        va_end(args);
        panic(buf);
}

void panic_interrupt(interrupt_frame *frame, const char *msg) {
        char buf[1024];
        sprintf(buf, "%s\ninterrupt frame:\nrip = 0x%x, cs = 0x%x, rflags = 0x%x\nrsp = 0x%x, ss = 0x%x", msg, frame->rip, frame->cs, frame->rflags, frame->rsp, frame->ss);
        panic(buf);
}

void panicf_interrupt(interrupt_frame *frame, const char *fmt, ...) {
        va_list args;
        va_start(args, fmt);
        char buf[1024];
        vasprintf(buf, fmt, args);
        va_end(args);
        panic_interrupt(frame, buf);
}
