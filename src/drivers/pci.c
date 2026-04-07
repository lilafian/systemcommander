/* SPDX-License-Identifier: GPL-3.0-or-later
 * Defines functions for getting PCI info and finding PCI devices
 * Copyright (C) 2026 lilaf */

#include <syscom/pci.h>
#include <syscom/acpi.h>
#include <syscom/page_map.h>
#include <syscom/log.h>
#include <syscom/string.h>

const char* PCI_DEVICE_CLASSES[] = {
    "Unclassified",
    "Mass Storage Controller",
    "Network Controller",
    "Display Controller",
    "Multimedia Controller",
    "Memory Controller",
    "Bridge Device",
    "Simple Communication Controller",
    "Base System Peripheral",
    "Input Device Controller",
    "Docking Station", 
    "Processor",
    "Serial Bus Controller",
    "Wireless Controller",
    "Intelligent Controller",
    "Satellite Communication Controller",
    "Encryption Controller",
    "Signal Processing Controller",
    "Processing Accelerator",
    "Non Essential Instrumentation"
};

const char* pci_get_vendor_name(uint16_t vendor_id) {
    switch (vendor_id) {
        case 0x8086:
            return "Intel Corporation";
        case 0x1022:
            return "Advanced Micro Devices, Inc.";
        case 0x10DE:
            return "NVIDIA Corporation";
        case 0x1234:
            return "QEMU";
    }
    return itoa(vendor_id, 16);
}

const char* pci_get_device_name(uint16_t vendor_id, uint16_t device_id) {
    switch (vendor_id){
            case 0x8086: // Intel
                switch(device_id){
                    case 0x29C0:
                        return "Express DRAM Controller";
                    case 0x2918:
                        return "LPC Interface Controller";
                    case 0x2922:
                        return "6 port SATA Controller [AHCI mode]";
                    case 0x2930:
                        return "SMBus Controller";
                }
        }
        return itoa(device_id, 16);
}

const char* pci_get_mass_storage_controller_subclass_name(uint8_t subclass_code) {
    switch (subclass_code){
        case 0x00:
            return "SCSI Bus Controller";
        case 0x01:
            return "IDE Controller";
        case 0x02:
            return "Floppy Disk Controller";
        case 0x03:
            return "IPI Bus Controller";
        case 0x04:
            return "RAID Controller";
        case 0x05:
            return "ATA Controller";
        case 0x06:
            return "Serial ATA";
        case 0x07:
            return "Serial Attached SCSI";
        case 0x08:
            return "Non-Volatile Memory Controller";
        case 0x80:
            return "Other";
    }
    return itoa(subclass_code, 16);
}

const char* pci_get_serial_bus_controller_subclass_name(uint8_t subclass_code) {
    switch (subclass_code){
        case 0x00:
            return "FireWire (IEEE 1394) Controller";
        case 0x01:
            return "ACCESS Bus";
        case 0x02:
            return "SSA";
        case 0x03:
            return "USB Controller";
        case 0x04:
            return "Fibre Channel";
        case 0x05:
            return "SMBus";
        case 0x06:
            return "Infiniband";
        case 0x07:
            return "IPMI Interface";
        case 0x08:
            return "SERCOS Interface (IEC 61491)";
        case 0x09:
            return "CANbus";
        case 0x80:
            return "SerialBusController - Other";
    }
    return itoa(subclass_code, 16);
}

const char* pci_get_bridge_device_subclass_name(uint8_t subclass_code) {
    switch (subclass_code) {
        case 0x00:
            return "Host Bridge";
        case 0x01:
            return "ISA Bridge";
        case 0x02:
            return "EISA Bridge";
        case 0x03:
            return "MCA Bridge";
        case 0x04:
            return "PCI-to-PCI Bridge";
        case 0x05:
            return "PCMCIA Bridge";
        case 0x06:
            return "NuBus Bridge";
        case 0x07:
            return "CardBus Bridge";
        case 0x08:
            return "RACEway Bridge";
        case 0x09:
            return "PCI-to-PCI Bridge";
        case 0x0a:
            return "InfiniBand-to-PCI Host Bridge";
        case 0x80:
            return "Other";
    }
    return itoa(subclass_code, 16);
}

const char* pci_get_subclass_name(uint8_t class_code, uint8_t subclass_code) {
    switch (class_code){
        case 0x01:
            return pci_get_mass_storage_controller_subclass_name(subclass_code);
        case 0x03:
            switch (subclass_code){
                case 0x00:
                    return "VGA Compatible Controller";
            }
        case 0x06:
            return pci_get_bridge_device_subclass_name(subclass_code);
        case 0x0C:
            return pci_get_serial_bus_controller_subclass_name(subclass_code);
    }
    return itoa(subclass_code, 16);
}

const char* pci_get_program_interface_name(uint8_t class_code, uint8_t subclass_code, uint8_t program_interface) {
    switch (class_code) {
        case 0x01:
            switch (subclass_code) {
                case 0x06:
                    switch (program_interface) {
                        case 0x00:
                            return "Vendor Specific Interface";
                        case 0x01:
                            return "AHCI 1.0";
                        case 0x02:
                            return "Serial Storage Bus";
                    }
            }
        case 0x03:
            switch (subclass_code) {
                case 0x00:
                    switch (program_interface) {
                        case 0x00:
                            return "VGA Controller";
                        case 0x01:
                            return "8514-Compatible Controller";
                    }
            }
        case 0x0C:
            switch (subclass_code) {
                case 0x03:
                    switch (program_interface) {
                        case 0x00:
                            return "UHCI Controller";
                        case 0x10:
                            return "OHCI Controller";
                        case 0x20:
                            return "EHCI (USB2) Controller";
                        case 0x30:
                            return "XHCI (USB3) Controller";
                        case 0x80:
                            return "Unspecified";
                        case 0xFE:
                            return "USB Device (Not a Host Controller)";
                    }
            }    
    }
    return itoa(program_interface, 16);
}

void function_enumerate(uint64_t device_address, uint64_t function) {
    uint64_t offset = function << 12;
    uint64_t function_addr = device_address + offset;
    map_virtual_memory(kernel_pml4, PAGE_ALIGN(function_addr), PAGE_ALIGN(function_addr), PAGE_RW | PAGE_USER);

    pci_device_header* device_header = (pci_device_header*)function_addr;

    if (device_header->device_id == 0 || device_header->device_id == 0xffff) return;

    logf("[pci_enumerate/function_enumerate] %s %s - %s - %s - %s\n", 
            pci_get_vendor_name(device_header->vendor_id), 
            pci_get_device_name(device_header->vendor_id, device_header->device_id), 
            PCI_DEVICE_CLASSES[device_header->device_class], 
            pci_get_subclass_name(device_header->device_class, device_header->device_subclass), 
            pci_get_program_interface_name(device_header->device_class, device_header->device_subclass, device_header->program_interface));
}

void device_enumerate(uint64_t bus_address, uint64_t device) {
    uint64_t offset = device << 15;
    uint64_t device_addr = bus_address + offset;
    map_virtual_memory(kernel_pml4, PAGE_ALIGN(device_addr), PAGE_ALIGN(device_addr), PAGE_RW | PAGE_USER);

    pci_device_header* device_header = (pci_device_header*)device_addr;

    if (device_header->device_id == 0 || device_header->device_id == 0xffff) return;

    for (uint64_t function = 0; function < 8; function++) {
        function_enumerate(device_addr, function);
    }
}

void bus_enumerate(uint64_t base_address, uint64_t bus) {
    uint64_t offset = bus << 20;
    uint64_t bus_addr = base_address + offset;
    map_virtual_memory(kernel_pml4, PAGE_ALIGN(bus_addr), PAGE_ALIGN(bus_addr), PAGE_RW | PAGE_USER);

    pci_device_header* device_header = (pci_device_header*)bus_addr;

    if (device_header->device_id == 0 || device_header->device_id == 0xffff) return;

    for (uint64_t device = 0; device < 32; device++) {
        device_enumerate(bus_addr, device);
    }
}

void pci_enumerate(mcfg_header* mcfg) {
    int entry_count = ((mcfg->header.size) - sizeof(mcfg_header)) / sizeof(acpi_device_config);

    for (int i = 0; i < entry_count; i++) {
        acpi_device_config* new_dev_config  = (acpi_device_config*)((uint64_t)mcfg + sizeof(mcfg_header) + (sizeof(acpi_device_config) * i));
        for (uint64_t bus = new_dev_config->start_bus; bus < new_dev_config->end_bus; bus++) {
            bus_enumerate(new_dev_config->base, bus);
        }
    }
}

