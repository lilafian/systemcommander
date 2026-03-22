#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <limine.h>
#include <syscom/psf.h>
#include <syscom/console.h>
#include <syscom/log.h>
#include <syscom/memory.h>

__attribute__((used, section(".limine_requests")))
static volatile uint64_t limine_base_revision[] = LIMINE_BASE_REVISION(5);

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

void kenter() {
        if (LIMINE_BASE_REVISION_SUPPORTED(limine_base_revision) == false) {
                halt();
        }

        if (framebuffer_request.response == NULL
                        || framebuffer_request.response->framebuffer_count < 1) {
                halt();
        }

        struct limine_framebuffer *framebuffer = framebuffer_request.response->framebuffers[0];

        struct limine_file **modules = module_request.response->modules;
        struct limine_file *font_module = modules[0]; // If more modules get added pls change!
        psf2_header_t *font = (psf2_header_t *)(font_module->address);

        console_t console;
        console_init(&console, framebuffer, font, 0xffaaaaaa, 0x00000000);

        set_log_console(&console);
        log("System Commander version 0.1.0 (Vientiane)\n");
        logf("Detected %d MiB of usable memory\n", get_memory_size(memmap_request.response) / 1024 / 1024);

        halt();
}
