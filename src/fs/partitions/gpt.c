/* SPDX-License-Identifier: GPL-3.0-or-later
 * Defines functions for working with GPT disks.
 * Copyright (C) 2026 lilaf */

#include <syscom/gpt.h>
#include <syscom/string.h>
#include <syscom/heap.h>

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
