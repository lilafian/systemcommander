/* SPDX-License-Identifier: GPL-3.0-or-later
 * Defines functions for panicking the kernel.
 * Copyright (C) 2026 lilaf */

#include <syscom/panic.h>
#include <syscom/log.h>
#include <syscom/fmt.h>

void cr_dump(char *dest) {
        uint64_t cr0, cr2, cr3, cr4;
        asm volatile (
                        "movq %%cr0, %0\n"
                        "movq %%cr2, %1\n"
                        "movq %%cr3, %2\n"
                        "movq %%cr4, %3\n"
                        : "=r"(cr0), "=r"(cr2), "=r"(cr3), "=r"(cr4)
                     );

        sprintf(dest, "cr0 = 0x%x, cr2 = 0x%x, cr3 = 0x%x, cr4 = 0x%x\n", cr0, cr2, cr3, cr4);
}       

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
        char cr_dump_buf[1024];
        cr_dump(cr_dump_buf);
        sprintf(buf, "%s\ninterrupt frame:\nrip = 0x%x, cs = 0x%x, rflags = 0x%x\nrsp = 0x%x, ss = 0x%x\ncontrol registers:\n%s", msg, frame->rip, frame->cs, frame->rflags, frame->rsp, frame->ss, cr_dump_buf);
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
