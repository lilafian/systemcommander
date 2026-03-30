#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <limine.h>
#include <syscom/psf.h>
#include <syscom/console.h>
#include <syscom/log.h>
#include <syscom/memory.h>
#include <syscom/phys_allocator.h>
#include <syscom/serial.h>
#include <syscom/page_map.h>
#include <syscom/heap.h>

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

        logf("[kenter] Detected %d MiB of memory\n", get_memory_size(memmap_request.response) / 1024 / 1024);
        
        read_memory_map(memmap_request.response);
        logf("[kenter] Free: %d KiB\n         Used: %d KiB\n         Reserved: %d KiB\n", get_free_memory() / 1024, get_used_memory() / 1024, get_reserved_memory() / 1024);

        uint64_t phys_pml4 = 0;
        asm volatile ("movq %%cr3, %0" : "=r"(phys_pml4));
        if (!phys_pml4) {
                log("<FATAL> [kenter] Failed to retrieve page map from cr3\n");
                halt();
        }
        uint64_t virt_pml4 = phys_pml4 + hhdm_offset;
        kernel_pml4 = (page_table *)virt_pml4;
        logf("[kenter] Found page map (kernel_pml4) at 0x%x (0x%x)\n", phys_pml4, kernel_pml4);

        heap_init((void*)0x100000000000, 0x10);

        halt();
}
