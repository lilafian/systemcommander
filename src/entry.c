/* SPDX-License-Identifier: GPL-3.0-or-later
 * Kernel entry function and bootloader requests.
 * Copyright (C) 2026 lilaf */

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <limine.h>
#include <syscom/psf.h>
#include <syscom/console.h>
#include <syscom/log.h>
#include <syscom/memory.h>
#include <syscom/phys_allocator.h>
#include <syscom/drivers/serial.h>
#include <syscom/page_map.h>
#include <syscom/heap.h>
#include <syscom/panic.h>
#include <syscom/gdt.h>
#include <syscom/interrupts.h>
#include <syscom/acpi.h>
#include <syscom/pci.h>
#include <syscom/drivers/ahci.h>

__attribute__((used, section(".limine_requests")))
static volatile uint64_t limine_base_revision[] = LIMINE_BASE_REVISION(6);

__attribute__((used, section(".limine_requests")))
static volatile struct limine_framebuffer_request framebuffer_request = {
        .id = LIMINE_FRAMEBUFFER_REQUEST_ID,
        .revision = 0
};

__attribute__((used, section(".limine_requests")))
static volatile struct limine_memmap_request memmap_request = {
        .id = LIMINE_MEMMAP_REQUEST_ID,
        .revision = 0
};

__attribute__((used, section(".limine_requests")))
static volatile struct limine_hhdm_request hhdm_request = {
        .id = LIMINE_HHDM_REQUEST_ID,
        .revision = 0
};

static struct limine_internal_module font_module_descriptor = {
        .path = "deffont.psf",
        .string = "deffont",
        .flags = LIMINE_INTERNAL_MODULE_REQUIRED
};

struct limine_internal_module* modules[] = {
        &font_module_descriptor
};

__attribute__((used, section(".limine_requests")))
static volatile struct limine_module_request module_request = {
        .id = LIMINE_MODULE_REQUEST_ID,
        .revision = 1,
        .internal_module_count = 1,
        .internal_modules = modules
};

__attribute__((used, section(".limine_requests")))
static volatile struct limine_rsdp_request rsdp_request = {
        .id = LIMINE_RSDP_REQUEST_ID,
        .revision = 1
};

__attribute__((used, section(".limine_requests_start")))
static volatile uint64_t limine_requests_start_marker[] = LIMINE_REQUESTS_START_MARKER;

__attribute__((used, section(".limine_requests_end")))
static volatile uint64_t limine_requests_end_marker[] = LIMINE_REQUESTS_END_MARKER;

static void halt() {
        for (;;) { asm("hlt"); }
}

uint64_t hhdm_offset;
void kenter() {
        if (LIMINE_BASE_REVISION_SUPPORTED(limine_base_revision) == false) {
                halt();
        }

        set_log_mode(LOG_MODE_FRAMEBUFFER_SERIAL);

        bool can_use_fb = true;
        if (framebuffer_request.response == NULL
                        || framebuffer_request.response->framebuffer_count < 1) {
                set_log_mode(LOG_MODE_SERIAL_ONLY);
                can_use_fb = false;
        }

        hhdm_offset = hhdm_request.response->offset;

        if (can_use_fb) {
                struct limine_framebuffer *framebuffer = framebuffer_request.response->framebuffers[0];

                struct limine_file **modules = module_request.response->modules;
                struct limine_file *font_module = modules[0]; // If more modules get added pls change!
                psf2_header_t *font = (psf2_header_t *)(font_module->address);

                console console;
                console_init(&console, framebuffer, font, 0xffaaaaaa, 0x00000000);
                set_log_console(&console);
        }

        serial_init();

        log("System Commander version 0.1.0 (Vientiane)\n");

        logf("[init:kenter] Detected %d MiB of memory\n", get_memory_size(memmap_request.response) / 1024 / 1024);
        
        read_memory_map(memmap_request.response);
        logf("[init:kenter] Free: %d KiB\n              Used: %d KiB\n              Reserved: %d KiB\n", get_free_memory() / 1024, get_used_memory() / 1024, get_reserved_memory() / 1024);

        gdt_descriptor gdt_desc;
        gdt_desc.size = sizeof(gdt) - 1;
        gdt_desc.offset = (uint64_t)&default_gdt;
        load_gdt(&gdt_desc);

        idtr idtr_struct;
        idtr_struct.limit = 0x0fff;
        idtr_struct.offset = (uint64_t)request_page() + hhdm_offset;

        register_interrupt_handler(idtr_struct, 0x00, (uint64_t)inthdlr_division_error);
        register_interrupt_handler(idtr_struct, 0x05, (uint64_t)inthdlr_bound_range_exceeded);
        register_interrupt_handler(idtr_struct, 0x06, (uint64_t)inthdlr_invalid_opcode);
        register_interrupt_handler(idtr_struct, 0x07, (uint64_t)inthdlr_device_not_available);
        register_interrupt_handler(idtr_struct, 0x0A, (uint64_t)inthdlr_invalid_tss);
        register_interrupt_handler(idtr_struct, 0x0B, (uint64_t)inthdlr_segment_not_present);
        register_interrupt_handler(idtr_struct, 0x0C, (uint64_t)inthdlr_stackseg_fault);
        register_interrupt_handler(idtr_struct, 0x0D, (uint64_t)inthdlr_genprotect_fault);
        register_interrupt_handler(idtr_struct, 0x0E, (uint64_t)inthdlr_page_fault);
        register_interrupt_handler(idtr_struct, 0x10, (uint64_t)inthdlr_fp_exception);
        register_interrupt_handler(idtr_struct, 0x11, (uint64_t)inthdlr_alignment_check);
        register_interrupt_handler(idtr_struct, 0x13, (uint64_t)inthdlr_simd_fp_exception);
        register_interrupt_handler(idtr_struct, 0x14, (uint64_t)inthdlr_virtualization_exception);
        register_interrupt_handler(idtr_struct, 0x15, (uint64_t)inthdlr_conprotect_exception);
        register_interrupt_handler(idtr_struct, 0x1C, (uint64_t)inthdlr_hypervisor_inj_exception);
        register_interrupt_handler(idtr_struct, 0x1D, (uint64_t)inthdlr_vmm_communication_exception);
        register_interrupt_handler(idtr_struct, 0x1E, (uint64_t)inthdlr_security_exception);

        asm volatile ("lidt %0" :: "m" (idtr_struct));

        uint64_t phys_pml4 = 0;
        asm volatile ("movq %%cr3, %0" : "=r"(phys_pml4));
        if (!phys_pml4) {
                panic("[init:kenter] Failed to retrieve page map from cr3");
        }
        uint64_t virt_pml4 = phys_pml4 + hhdm_offset;
        kernel_pml4 = (page_table *)virt_pml4;
        logf("[init:kenter] Found page map (kernel_pml4) at 0x%x (0x%x)\n", phys_pml4, kernel_pml4);

        heap_init((void*)HEAP_START_VIRTUAL, 0x10);

        rsdp2 *rsdp = (rsdp2 *)rsdp_request.response->address;
        for (int i = 0; i < 0x8000; i += 0x1000) {
                map_virtual_memory(kernel_pml4, PAGE_ALIGN((uint64_t)rsdp + i), PAGE_ALIGN((uint64_t)rsdp + i), PAGE_RW);
        }

        sdt_header *xsdt = (sdt_header *)rsdp->xsdt_address;
        map_virtual_memory(kernel_pml4, PAGE_ALIGN((uint64_t)xsdt), PAGE_ALIGN((uint64_t)xsdt), PAGE_RW | PAGE_USER);
        mcfg_header *mcfg = (mcfg_header *)acpi_find_table(xsdt, "MCFG");
        map_virtual_memory(kernel_pml4, PAGE_ALIGN((uint64_t)mcfg), PAGE_ALIGN((uint64_t)mcfg), PAGE_RW | PAGE_USER);
        if (mcfg == NULL) {
                panic("[init:kenter] Failed to find MCFG table");
        }
        pci_enumerate(mcfg);

        ahci_driver_info *ahci_driver;
        for (int i = 0; i < pci_driver_count; i++) {
                pci_driver driver = pci_drivers[i];
                if (driver.type == PCI_DRIVER_TYPE_AHCI) {
                        ahci_driver = driver.info;
                }
        }
        if (!ahci_driver) panic("[init:kenter] Failed to find AHCI driver\nOther types of disk drivers are not available yet");

        halt();
}
