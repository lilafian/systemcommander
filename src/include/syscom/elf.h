/* SPDX-License-Identifier: GPL-3.0-or-later
 * Declares structures and functions for loading and using ELF files.
 * Copyright (C) 2026 lilaf */

#pragma once
#include <stdint.h>
#include <stddef.h>

#define ELF_MAGIC "\x7f" "ELF"

#define ELF_BITNESS_32 1
#define ELF_BITNESS_64 2

#define ELF_ENDIANNESS_LITTLE 1
#define ELF_ENDIANNESS_BIG 2

#define ELF_TYPE_RELOCATABLE 1
#define ELF_TYPE_EXECUTABLE 2
#define ELF_TYPE_SHARED 3
#define ELF_TYPE_CORE 4

#define PT_NULL 0
#define PT_LOAD 1
#define PT_DYNAMIC 2
#define PT_INTERP 3
#define PT_NOTE 4

#define PHDR_FLAG_EXEC 1
#define PHDR_FLAG_WRITE 2
#define PHDR_FLAG_READ 4

#define ELF_ISA_X86_64 0x3E

#define USER_STACK_TOP 0x00007fffffffe000
#define USER_STACK_SIZE 0x4000

typedef struct elf_header {
        uint32_t magic;
        uint8_t bitness;
        uint8_t endianness;
        uint8_t header_version;
        uint8_t abi; // 0 for sysv
        uint64_t _unused;
        uint16_t type;
        uint16_t isa;
        uint32_t elf_version; // 1
        uint64_t entry;
        uint64_t phdr_table_offset;
        uint64_t shdr_table_offset;
        uint32_t flags; // ignored for x86
        uint16_t header_size;
        uint16_t phdr_entry_size;
        uint16_t phdr_entry_count;
        uint16_t shdr_entry_size;
        uint16_t shdr_entry_count;
        uint16_t shdr_string_table_index;
}__attribute__((packed)) elf_header;

typedef struct elf_program_header {
        uint32_t type;
        uint32_t flags;
        uint64_t data_offset;
        uint64_t virtual_base;
        uint64_t physical_base; // ?
        uint64_t size_file;
        uint64_t size_memory;
        uint64_t alignment;
}__attribute__((packed)) elf_program_header;

void elf_load(char *data, size_t data_size);
