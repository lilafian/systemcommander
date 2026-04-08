/* SPDX-License-Identifier: GPL-3.0-or-later
 * Declares structures and functions for working with GPT disks.
 * Copyright (C) 2026 lilaf */

#pragma once

#include <stdbool.h>
#include <syscom/string.h>
#include <syscom/drivers/ahci.h>

#define GPT_PENTRY_ATTRIB_FW_REQUIRED 1

typedef struct gpt_header {
        char signature[8];
        uint32_t revision;
        uint32_t header_size;
        uint32_t checksum;
        uint32_t reserved;
        uint64_t header_lba;
        uint64_t alternate_header_lba;
        uint64_t first_usable;
        uint64_t last_usable;
        char guid[16];
        uint64_t entry_array_start;
        uint32_t entry_count;
        uint32_t entry_size;
        uint32_t entry_array_checksum;
} gpt_header;

typedef struct gpt_partition_entry {
        char type_guid[16];
        char guid[16];
        uint64_t start_sector;
        uint64_t end_sector;
        uint64_t attributes;
        char16 name[36];
} gpt_partition_entry;

bool is_gpt(ahci_port *disk);
bool gpt_is_unused_partition(gpt_partition_entry *partition);

bool gpt_read_partition(ahci_port *disk, gpt_partition_entry *partition, uint64_t start_sector, uint32_t count, void *buffer);
