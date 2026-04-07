/* SPDX-License-Identifier: GPL-3.0-or-later
 * Declares functions and constants for serial communication.
 * Copyright (C) 2026 lilaf */

#pragma once

#define COM1 0x3F8
#define COM2 0x2F8
#define COM3 0x3E8
#define COM4 0x2E8

#define SERIAL_IRQ 4

void serial_init();
void serial_enable(int device);
void serial_send(int device, char out);

int serial_rcvd(int device);
char serial_recv(int device);
char serial_recv_async(int device);
int serial_transmit_empty(int device);
