void _start() {
        __asm__ volatile("outb %0, %1" :: "a"((char)'A'), "Nd"((short)0x3F8));
}
