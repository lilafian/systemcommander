; SPDX-License-Identifier: GPL-3.0-or-later
; Defines a function to pass to a higher-level syscall handler
; Copyright (C) 2026 lilaf

bits 64
default rel
extern stack_top
extern syscall_handler

section .bss
user_rsp_saved: resq 1

section .text
global syscall_entry
syscall_entry:
    mov [user_rsp_saved], rsp
    lea rsp, [stack_top]
    push rcx            ; user rip
    push r11            ; user rflags
    mov rcx, r10
    push rbp
    push rbx
    push r12
    push r13
    push r14
    push r15
    mov r9,  r8
    mov r8,  r10
    mov rcx, rdx
    mov rdx, rsi
    mov rsi, rdi
    mov rdi, rax
    call syscall_handler ; c handler
    pop r15
    pop r14
    pop r13
    pop r12
    pop rbx
    pop rbp
    pop r11
    pop rcx
    mov rsp, [user_rsp_saved]
    o64 sysret
