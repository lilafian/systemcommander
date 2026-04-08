/* SPDX-License-Identifier: GPL-3.0-or-later
 * Defines structures and functions for a simple AHCI driver.
 * Copyright (C) 2026 lilaf */

#include <syscom/drivers/ahci.h>
#include <syscom/log.h>
#include <syscom/page_map.h>
#include <syscom/heap.h>
#include <syscom/stdmemory.h>
#include <syscom/phys_allocator.h>

hba_port_type ahci_get_port_type(hba_port *port) {
        uint32_t sata_status = port->sata_status;
        uint8_t if_pm = (sata_status >> 8) & 0b111;
        uint8_t dev_det = sata_status & 0b111;

        if (dev_det != HBA_PORT_DEVICE_PRESENT) return HBA_PORT_NOTYPE;
        if (if_pm != HBA_PORT_IPM_ACTIVE) return HBA_PORT_NOTYPE;

        switch (port->signature) {
                case SATA_SIGNATURE_ATAPI:
                        return HBA_PORT_SATAPI;
                case SATA_SIGNATURE_ATA:
                        return HBA_PORT_SATA;
                case SATA_SIGNATURE_PM:
                        return HBA_PORT_PM;
                case SATA_SIGNATURE_SEMB:
                        return HBA_PORT_SEMB;
                default:
                        return HBA_PORT_NOTYPE;
        }
}

void ahci_probe_ports(ahci_driver_info *info) {
        uint32_t impl = info->abar->ports_implemented;
        for (int i = 0; i < 32; i++) {
                if (impl & (1 << i)) {
                        hba_port_type port_type = ahci_get_port_type(&info->abar->ports[i]);
                        if (port_type == HBA_PORT_SATA) logf("[ahci:ahci_probe_ports] Found SATA device on port %d\n", i);
                        if (port_type == HBA_PORT_SATAPI) logf("[ahci:ahci_probe_ports] Found SATAPI device on port %d\n", i);

                        if (port_type == HBA_PORT_SATA || port_type == HBA_PORT_SATAPI) {
                                info->ports[info->port_count] = malloc(sizeof(ahci_port));
                                info->ports[info->port_count]->type = port_type;
                                info->ports[info->port_count]->hba = &info->abar->ports[i];
                                info->ports[info->port_count]->port_index = info->port_count;
                                info->port_count++;
                        }
                }
        }
}

void ahci_configure_port(ahci_port *port) {
        ahci_port_stop_cmd(port);

        void *new_cl_base = request_page();
        map_virtual_memory(kernel_pml4, PAGE_ALIGN((uint64_t)new_cl_base), PAGE_ALIGN((uint64_t)new_cl_base), PAGE_RW);
        port->hba->command_list_base_lo = (uint32_t)(uint64_t)new_cl_base;
        port->hba->command_list_base_up = (uint32_t)((uint64_t)new_cl_base >> 32);
        memset(new_cl_base, 0, 0x1000);

        void *new_fis_base = request_page();
        map_virtual_memory(kernel_pml4, PAGE_ALIGN((uint64_t)new_fis_base), PAGE_ALIGN((uint64_t)new_fis_base), PAGE_RW);
        port->hba->fis_base_lo = (uint32_t)(uint64_t)new_fis_base;
        port->hba->fis_base_up = (uint32_t)((uint64_t)new_fis_base >> 32);
        memset(new_fis_base, 0, 0x1000);

        hba_command_header *command_header = (hba_command_header *)((uint64_t)port->hba->command_list_base_lo + ((uint64_t)port->hba->command_list_base_up << 32));
        for (int i = 0; i < 32; i++) {
                command_header[i].prdt_length = 8;
                void *command_table = request_page();
                map_virtual_memory(kernel_pml4, PAGE_ALIGN((uint64_t)command_table), PAGE_ALIGN((uint64_t)command_table), PAGE_RW);

                uint64_t addr = (uint64_t)command_table + (i << 8);
                command_header[i].cmd_table_base_lo = (uint32_t)addr;
                command_header[i].cmd_table_base_up = (uint32_t)((uint64_t)addr >> 32);
                memset(command_table, 0, 1024);
        }

        ahci_port_start_cmd(port);
}

void ahci_port_start_cmd(ahci_port *port) {
        while (port->hba->command_status & HBA_PXCMD_CR);

        port->hba->command_status |= HBA_PXCMD_FRE;
        port->hba->command_status |= HBA_PXCMD_ST;
}

void ahci_port_stop_cmd(ahci_port *port) {
        port->hba->command_status &= ~HBA_PXCMD_ST;
        port->hba->command_status &= ~HBA_PXCMD_FRE;

        for (;;) {
                if (port->hba->command_status & HBA_PXCMD_FR) continue;
                if (port->hba->command_status & HBA_PXCMD_CR) continue;
                break;
        }
}

bool ahci_read(ahci_port *port, uint64_t start_sector, uint32_t count, void *buffer) {
        uint64_t spin = 0;
        while ((port->hba->task_file_data & (ATA_DEVICE_BUSY | ATA_DEVICE_DRQ)) && spin < 1000000) {
                spin++;
        }

        if (spin >= 1000000) return false;

        uint32_t sector_lo = (uint32_t)start_sector;
        uint32_t sector_up = (uint32_t)(start_sector >> 32);

        port->hba->interrupt_status = (uint32_t)-1;

        hba_command_header *command_header = (hba_command_header *)((uint64_t)port->hba->command_list_base_lo + ((uint64_t)port->hba->command_list_base_up << 32));
        command_header->flags = (sizeof(fis_reg_h2d) / sizeof(uint32_t)) & HBA_CMD_CFL_MASK;
        command_header->flags &= ~HBA_CMD_WRITE;
        command_header->prdt_length = 1;

        hba_command_table *command_table = (hba_command_table *)((uint64_t)command_header->cmd_table_base_lo + ((uint64_t)command_header->cmd_table_base_up << 32));
        memset(command_table, 0, sizeof(hba_command_table) + (command_header->prdt_length - 1) * sizeof(hba_prdt_entry));

        command_table->prdt_entries[0].data_base_lo = (uint32_t)(uint64_t)buffer;
        command_table->prdt_entries[0].data_base_up = (uint32_t)((uint64_t)buffer >> 32);
        command_table->prdt_entries[0].flags = ((count << 9) - 1) & HBA_PRDTE_BYTE_COUNT_MASK;
        command_table->prdt_entries[0].flags |= HBA_PRDTE_INTERRUPT_ON_COMPLETION;
        
        fis_reg_h2d *cmd_fis = (fis_reg_h2d *)(&command_table->command_fis);
        cmd_fis->fis_type = FIS_TYPE_REG_H2D;
        cmd_fis->flags |= FIS_REG_H2D_COMMAND_CONTROL;
        cmd_fis->command = ATA_CMD_READ_DMA_EX;

        cmd_fis->lba0 = (uint8_t)sector_lo;
        cmd_fis->lba1 = (uint8_t)(sector_lo >> 8);
        cmd_fis->lba2 = (uint8_t)(sector_lo >> 16);
        cmd_fis->lba3 = (uint8_t)sector_up;
        cmd_fis->lba4 = (uint8_t)(sector_up >> 8);
        cmd_fis->lba5 = (uint8_t)(sector_up >> 16);

        cmd_fis->device_register = 1 << 6;

        cmd_fis->count_lo = count & 0xFF;
        cmd_fis->count_up = (count >> 8) & 0xFF;

        port->hba->command_issue = 1;

        while (true) {
                if (port->hba->command_issue == 0) break;
                if (port->hba->interrupt_status & HBA_PXIS_TFES) return false;
        }

        return true;
}

bool ahci_read_virt(ahci_port *port, uint64_t start_sector, uint32_t count, void *buffer) {
        port->buffer = request_pages(count % 2 == 0 ? count : count + 1);
        if (!port->buffer) return false;
        map_virtual_memory(kernel_pml4, (uint64_t)port->buffer, (uint64_t)port->buffer, PAGE_RW);
        memset(port->buffer, 0, count * 512);

        bool success = ahci_read(port, start_sector, count, port->buffer);
        if (!success) return false;
        
        memcpy(buffer, port->buffer, count * 512);
        return true;
}

ahci_driver_info *ahci_init(pci_device_header *pci_base) {
        logf("[ahci:ahci_init] Initializing AHCI driver at PCI base 0x%x\n", pci_base);

        uint64_t abar_phys = (uint64_t)((pci_general_device *)pci_base)->bar[5];
        map_virtual_memory(kernel_pml4, AHCI_VIRT_ABAR_BASE + PAGE_ALIGN(abar_phys), PAGE_ALIGN(abar_phys), PAGE_RW);
        hba_memory *abar = (hba_memory *)(AHCI_VIRT_ABAR_BASE + PAGE_ALIGN(abar_phys));
        
        ahci_driver_info *info = malloc(sizeof(ahci_driver_info));
        memset(info, 0, sizeof(ahci_driver_info));
        info->abar = abar;

        ahci_probe_ports(info);

        for (int i = 0; i < info->port_count; i++) {
                ahci_port *port = info->ports[i];
                ahci_configure_port(port);
        }

        return info;
}
