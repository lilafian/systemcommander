/* SPDX-License-Identifier: GPL-3.0-or-later
 * Declares structures and functions for a simple AHCI driver.
 * Copyright (C) 2026 lilaf */

#pragma once
#include <stdint.h>
#include <stdbool.h>
#include <syscom/pci.h>

#define AHCI_VIRT_ABAR_BASE 0xffff800010000000

typedef uint8_t hba_port_type;
#define HBA_PORT_NOTYPE 0
#define HBA_PORT_SATA 1
#define HBA_PORT_SEMB 2
#define HBA_PORT_PM 3
#define HBA_PORT_SATAPI 4

#define HBA_PORT_DEVICE_PRESENT 0x03
#define HBA_PORT_IPM_ACTIVE 0x01

#define SATA_SIGNATURE_ATAPI 0xeb140101
#define SATA_SIGNATURE_ATA 0x00000101
#define SATA_SIGNATURE_SEMB 0xc33c0101
#define SATA_SIGNATURE_PM 0x96690101

#define ATA_CMD_READ_DMA_EX 0x25
#define ATA_CMD_WRITE_DMA_EX 0x35

#define ATA_DEVICE_BUSY 0x80
#define ATA_DEVICE_DRQ 0x08

#define HBA_PXCMD_CR 0x8000
#define HBA_PXCMD_FRE 0x0010
#define HBA_PXCMD_ST 0x0001
#define HBA_PXCMD_FR 0x4000

#define HBA_PXIS_TFES (1 << 30)

typedef struct hba_port {
        uint32_t command_list_base_lo;
        uint32_t command_list_base_up;
        uint32_t fis_base_lo;
        uint32_t fis_base_up;
        uint32_t interrupt_status;
        uint32_t interrupt_enable;
        uint32_t command_status;
        uint32_t _reserved0;
        uint32_t task_file_data;
        uint32_t signature;
        uint32_t sata_status;
        uint32_t sata_control;
        uint32_t sata_error;
        uint32_t sata_active;
        uint32_t command_issue;
        uint32_t sata_notification;
        uint32_t fis_switch_control;
        uint32_t _reserved1[11];
        uint32_t vendor[4];
} hba_port;

typedef struct hba_memory {
        uint32_t capabilities;
        uint32_t gh_control;
        uint32_t interrupt_status;
        uint32_t ports_implemented;
        uint32_t version;
        uint32_t ccc_control;
        uint32_t ccc_ports;
        uint32_t em_location;
        uint32_t em_control;
        uint32_t cap_ext;
        uint32_t handoff_ctl_sts;
        uint8_t _reserved[0x74];
        uint8_t vendor[0x60];
        hba_port ports[32];
} hba_memory;

#define HBA_CMD_CFL_MASK 0x001f
#define HBA_CMD_ATAPI (1 << 5)
#define HBA_CMD_WRITE (1 << 6)
#define HBA_CMD_PREFETCHABLE (1 << 7)
#define HBA_CMD_RESET (1 << 8)
#define HBA_CMD_BIST (1 << 9)
#define HBA_CMD_CLEAR_BUSY (1 << 10)
#define HBA_CMD_PORT_MULT_MASK 0xF000
#define HBA_CMD_PORT_MULT_SHIFT 12

typedef struct hba_command_header {
        uint16_t flags;
        uint16_t prdt_length;
        uint32_t prdb_count;
        uint32_t cmd_table_base_lo;
        uint32_t cmd_table_base_up;
        uint32_t _reserved[4];
} hba_command_header;

#define HBA_PRDTE_BYTE_COUNT_MASK 0x3FFFFF
#define HBA_PRDTE_INTERRUPT_ON_COMPLETION (1 << 31)

typedef struct hba_prdt_entry {
        uint32_t data_base_lo;
        uint32_t data_base_up;
        uint32_t _reserved0;
        uint32_t flags;
} hba_prdt_entry;

typedef struct hba_command_table {
        uint8_t command_fis[64];
        uint8_t atapi_command[16];
        uint8_t _reserved[48];
        hba_prdt_entry prdt_entries[1];
} hba_command_table;

#define FIS_TYPE_REG_H2D 0x27
#define FIS_TYPE_REG_D2H 0x34
#define FIS_TYPE_DMA_ACT 0x39
#define FIS_TYPE_DMA_SETUP 0x41
#define FIS_TYPE_DATA 0x46
#define FIS_TYPE_BIST 0x58
#define FIS_TYPE_PIO_SETUP 0x5F
#define FIS_TYPE_DEV_BITS 0xA1

#define FIS_REG_H2D_PORT_MULT_MASK 0xF
#define FIS_REG_H2D_COMMAND_CONTROL (1 << 7)

typedef struct fis_reg_h2d {
        uint8_t fis_type;
        uint8_t flags;
        uint8_t command;
        uint8_t feature_lo;
        uint8_t lba0;
        uint8_t lba1;
        uint8_t lba2;
        uint8_t device_register;
        uint8_t lba3;
        uint8_t lba4;
        uint8_t lba5;
        uint8_t feature_up;
        uint8_t count_lo;
        uint8_t count_up;
        uint8_t iso_cmd_completion;
        uint8_t control;
        uint8_t _reserved[4];
} fis_reg_h2d;

typedef struct ahci_port {
        hba_port *hba;
        hba_port_type type;
        uint8_t *buffer;
        uint8_t port_index;
} ahci_port;

typedef struct ahci_driver_info {
        hba_memory *abar;
        ahci_port *ports[32];
        uint8_t port_count;
} ahci_driver_info;

ahci_driver_info *ahci_init(pci_device_header *pci_base);
hba_port_type ahci_get_port_type(hba_port *port);

void ahci_port_start_cmd(ahci_port *port);
void ahci_port_stop_cmd(ahci_port *port);

bool ahci_read(ahci_port *port, uint64_t start_sector, uint32_t count, void *buffer);
bool ahci_read_virt(ahci_port *port, uint64_t start_sector, uint32_t count, void *buffer);
bool ahci_write(ahci_port *port, uint64_t start_sector, uint32_t count, void *buffer);
bool ahci_write_virt(ahci_port *port, uint64_t start_sector, uint32_t count, void *buffer);
