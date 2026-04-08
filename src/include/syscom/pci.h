/* SPDX-License-Identifier: GPL-3.0-or-later
 * Declares a structure and function for PCI devices.
 * Copyright (C) 2026 lilaf */

#pragma once

#include <stdint.h>
#include <syscom/acpi.h>

typedef uint16_t pci_driver_type;
#define PCI_DRIVER_TYPE_AHCI 0x0001

#define PCI_MAX_DRIVERS 512

typedef struct pci_device_header {
    uint16_t vendor_id;
    uint16_t device_id;
    uint16_t command;
    uint16_t status;
    uint8_t revision_id;
    uint8_t program_interface;
    uint8_t device_subclass;
    uint8_t device_class;
    uint8_t cache_line_size;
    uint8_t latency_timer;
    uint8_t header_type;
    uint8_t bist;
}__attribute__((packed)) pci_device_header;

typedef struct pci_general_device {
        pci_device_header header;
        uint32_t bar[6];
        uint32_t cardbus_cis;
        uint16_t subsys_vendor_id;
        uint16_t subsys_id;
        uint32_t expansion_rom;
        uint8_t capabilities_ptr;
        uint8_t _reserved0;
        uint16_t _reserved1;
        uint32_t _reserved2;
        uint8_t interrupt_line;
        uint8_t interrupt_pin;
        uint8_t min_grant;
        uint8_t max_latency;
}__attribute__((packed)) pci_general_device;

typedef struct pci_driver {
        pci_driver_type type;
        void *info;
} pci_driver;

void pci_enumerate(mcfg_header* mcfg);

extern pci_driver pci_drivers[];
extern int pci_driver_count;
