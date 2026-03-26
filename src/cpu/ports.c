#include <syscom/ports.h>

uint8_t port_inb(uint16_t port) {
        uint8_t response;
        asm volatile("inb %1, %0" : "=a"(response) : "dN"(port));
        return response;
}

void port_outb(uint16_t port, uint8_t data) {
        asm volatile("outb %1, %0" : : "dN"(port), "a"(data));
}
