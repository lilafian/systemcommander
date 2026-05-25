bits 64

global _start
_start:
        mov al, 'A'
        mov dx, 0x3f8
        out dx, al

        mov rax, 60
        mov rdi, 0
        syscall
