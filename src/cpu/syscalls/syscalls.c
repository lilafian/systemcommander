/* SPDX-License-Identifier: GPL-3.0-or-later
 * Defines functions for using syscalls
 * Copyright (C) 2026 lilaf */

#include <syscom/syscalls.h>
#include <syscom/msr.h>
#include <syscom/log.h>

extern void syscall_entry(void);

void init_syscalls() {
        uint64_t efer = read_msr(0xC0000080);
        write_msr(0xC0000080, efer | 1);

        write_msr(0xC0000081, ((uint64_t)0x20 << 48) | ((uint64_t)0x08 << 32)); // user cs = 0x20, kernel cs = 0x08

        write_msr(0xC0000082, (uint64_t)syscall_entry);

        write_msr(0xC0000084, (1 << 9));
}

uint64_t syscall_handler(uint64_t num, uint64_t rdi, uint64_t rsi, uint64_t rdx, uint64_t r10, uint64_t r8, uint64_t r9) {
        logf("[syscalls:syscall_handler] Got syscall #%d, rdi=%d, rsi=%d, rdx=%d, r10=%d, r8=%d, r9=%d\n", num, rdi, rsi, rdx, r10, r8, r9);
        if (num == 60) {
                logf("[syscalls:syscall_handler] sys_exit(%d)\n", rdi);
                for (;;) __asm__("hlt");
        }
        return 0;
}
