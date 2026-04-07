/* SPDX-License-Identifier: GPL-3.0-or-later
 * Declares structures and functions for ACPI information.
 * Copyright (C) 2026 lilaf */

#pragma once

#include <stdint.h>

typedef struct rsdp2 {
        unsigned char signature[8]; /* RSD PTR */
        uint8_t checksum;
        char oem_id[6];
        uint8_t revision;
        uint32_t rsdt_address;
        uint32_t size;
        uint64_t xsdt_address;
        uint8_t extended_checksum;
        uint8_t reserved[3];
}__attribute__((packed)) rsdp2;

typedef struct sdt_header {
        unsigned char signature[4];
        uint32_t size;
        uint8_t revision;
        uint8_t checksum;
        char oem_id[6];
        char oem_table_id[8];
        uint32_t oem_revision;
        char creator_id[4];
        uint32_t creator_revision;
}__attribute__((packed)) sdt_header;

typedef struct mcfg_header {
    sdt_header header;
    uint64_t reserved;
}__attribute__((packed)) mcfg_header;

typedef struct acpi_device_config {
    uint64_t base;
    uint16_t pci_segment_group;
    uint8_t start_bus;
    uint8_t end_bus;
    uint32_t reserved;
}__attribute__((packed)) acpi_device_config;

sdt_header* acpi_find_table(sdt_header *header, char *signature);
