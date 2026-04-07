/* SPDX-License-Identifier: GPL-3.0-or-later
 * Declares functions for sending/receiving data through ports.
 * Copyright (C) 2026 lilaf */

#pragma once

#include <stdint.h>

uint8_t port_inb(uint16_t port);
void port_outb(uint16_t port, uint8_t data);
