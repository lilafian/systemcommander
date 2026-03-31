#include <syscom/acpi.h>
#include <syscom/page_map.h>

sdt_header *acpi_find_table(sdt_header *header, char *signature) {
        int entry_count = (header->size - sizeof(sdt_header)) / 8;
        for (int i = 0; i < entry_count; i++) {
                sdt_header *new_header = (sdt_header *)*(uint64_t *)((uint64_t)header + sizeof(sdt_header) + (i * 8));
                map_virtual_memory(kernel_pml4, PAGE_ALIGN((uint64_t)new_header), PAGE_ALIGN((uint64_t)new_header), PAGE_RW | PAGE_USER);
                for (int j = 0; j < 4; j++) {
                        if (new_header->signature[i] != signature[i]) break;
                        if (j == 3) return new_header;
                }
        }
        return 0;
}
