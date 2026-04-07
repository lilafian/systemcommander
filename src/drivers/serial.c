/* SPDX-License-Identifier: GPL-3.0-or-later
 * Defines functions for sending/receiving data through a serial device.
 * Copyright (C) 2026 lilaf */

#include <syscom/serial.h>
#include <syscom/ports.h>

void serial_enable(int device) {
        port_outb(device + 1, 0x00);
        port_outb(device + 3, 0x80);
        port_outb(device + 0, 0x03);
        port_outb(device + 1, 0x00);
        port_outb(device + 3, 0x03);
        port_outb(device + 2, 0xc7);
        port_outb(device + 4, 0x0b);
}

void serial_init() {
        serial_enable(COM1);
        port_outb(COM1 + 1, 0x01);
}

void serial_send(int device, char out) {
        while (serial_transmit_empty(device) == 0);
        port_outb(device, out);
}

int serial_recieved(int device) {
        return port_inb(device + 5) & 1;
}

char serial_recieve(int device) {
        while (serial_recieved(device) == 0);
        return port_inb(device);
}

char serial_recieve_async(int device) {
        return port_inb(device);
}

int serial_transmit_empty(int device) {
        return port_inb(device + 5) & 0x20;
}
