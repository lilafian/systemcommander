; SPDX-License-Identifier: GPL-3.0-or-later
; Function for jumping to an arbitrary entry point in ring 3.
; Copyright (C) 2026 lilaf

bits 64

global jump_usermode
jump_usermode:
        ; rdi = entry
        ; rsi = userspace stack top
        push 0x2b
        push rsi
        push 0x202
        push 0x23
        push rdi
        iretq
