/* SPDX-License-Identifier: GPL-3.0-or-later
 * Defines functions for working with GPT disks.
 * Copyright (C) 2026 lilaf */

#include <syscom/gpt.h>
#include <syscom/string.h>
#include <syscom/heap.h>
#include <syscom/stdmemory.h>

gpt_partition gpt_partitions[255];
int gpt_partition_count = 0;

bool is_gpt(ahci_port *disk) {
        gpt_header *header = (gpt_header *)malloc(512);
        bool read_success = ahci_read_virt(disk, 1, 1, header);
        if (!read_success) return false;

        if (strncmp(header->signature, "EFI PART", 8) != 0) return false;
        return true;
}

bool gpt_is_unused_partition(gpt_partition_entry *partition) {
        return (strncmp(partition->type_guid, "\0", 16) == 0);
}

bool gpt_read_partition(ahci_port *disk, gpt_partition_entry *partition, uint64_t start_sector, uint32_t count, void *buffer) {
        uint64_t start = partition->start_sector + start_sector;
        return ahci_read_virt(disk, start, count, buffer);
}

bool gpt_write_partition(ahci_port *disk, gpt_partition_entry *partition, uint64_t start_sector, uint32_t count, void *buffer) {
        uint64_t start = partition->start_sector + start_sector;
        return ahci_write_virt(disk, start, count, buffer);
}

bool gpt_write_partition_offset(ahci_port *disk, gpt_partition_entry *partition, uint64_t start_sector, uint64_t byte_offset, uint32_t count, void *buffer, size_t write_size) {
        if (byte_offset + write_size > 512 * count) return false;

        uint64_t start = partition->start_sector + start_sector;

        char *read = malloc(512 * count);
        bool read_success = ahci_read_virt(disk, start, count, read);
        if (!read_success) {
                free(read);
                return false;
        }

        memcpy(read + byte_offset, buffer, write_size);

        bool write_status = ahci_write_virt(disk, start, count, read);
        free(read);
        return write_status;
}

void gpt_register_partition(ahci_port *ahci, gpt_partition_entry *partition) {
        if (gpt_partition_count >= 255) return;

        gpt_partition part_struct = {
                .ahci = ahci,
                .entry = partition
        };

        gpt_partitions[gpt_partition_count] = part_struct;

        gpt_partition_count++;
}
