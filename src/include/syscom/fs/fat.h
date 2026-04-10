/* SPDX-License-Identifier: GPL-3.0-or-later
 * Declares structures and functions for working with FAT filesystems.
 * Copyright (C) 2026 lilaf */

#pragma once

#include <stdbool.h>
#include <syscom/string.h>
#include <syscom/fs.h>
#include <syscom/gpt.h>

#define FAT_FILE_ATTRIB_READ_ONLY 0x01
#define FAT_FILE_ATTRIB_HIDDEN 0x02
#define FAT_FILE_ATTRIB_SYSTEM 0x04
#define FAT_FILE_ATTRIB_VOLUME_ID 0x08
#define FAT_FILE_ATTRIB_DIRECTORY 0x10
#define FAT_FILE_ATTRIB_ARCHIVE 0x20
#define FAT_FILE_ATTRIB_LONG_FILENAME 0x0F

#define FAT_TYPE_NONE 0
#define FAT_TYPE_FAT12 1
#define FAT_TYPE_FAT16 2
#define FAT_TYPE_FAT32 3

typedef uint8_t fat_type;

typedef struct fat_boot_sector {
        uint8_t boot_jmp[3];
        char oem_id[8];
        uint16_t bytes_per_sector;
        uint8_t sectors_per_cluster;
        uint16_t reserved_sectors;
        uint8_t fats;
        uint16_t root_directory_entries;
        uint16_t total_sectors;
        uint8_t media_descriptor_type;
        uint16_t sectors_per_fat;
        uint16_t sectors_per_track;
        uint16_t heads;
        uint32_t hidden_sectors;
        uint32_t large_sector_count;
        unsigned char extended[43]; // sizeof(fat32_extended_boot_sector) = 43
}__attribute__((packed)) fat_boot_sector;

typedef struct fat1216_extended_boot_sector {
        uint8_t drive_number;
        uint8_t _reserved;
        uint8_t signature;
        uint32_t volume_serial;
        char label[11];
        char sys_id[8];
} fat1216_extended_boot_sector;

typedef struct fat32_extended_boot_sector {
        uint32_t sectors_per_fat;
        uint16_t flags;
        uint16_t version;
        uint32_t root_cluster;
        uint16_t fs_info_sector;
        uint16_t backup_boot_sector;
        uint8_t _reserved0[12];
        uint8_t drive_number;
        uint8_t _reserved1;
        uint8_t signature;
        uint32_t volume_serial;
        char label[11];
        char sys_id[8];
} fat32_extended_boot_sector;

typedef struct fat32_fsinfo {
        uint32_t signature0;
        uint8_t _reserved[480];
        uint32_t signature1;
        uint32_t last_free_cluster_count;
        uint32_t start_available_search;
} fat32_fsinfo;

#define FAT_TIME_HOUR 0b1111100000000000
#define FAT_TIME_MIN 0b0000011111100000
#define FAT_TIME_SEC 0b0000000000011111 // MULTIPLY BY 2
#define FAT_DATE_YEAR 0b1111111000000000
#define FAT_DATE_MONTH 0b0000000111100000
#define FAT_DATE_DAY 0b0000000000011111

typedef struct fat_file {
        char name[8];
        char extension[3];
        uint8_t attributes;
        uint8_t _reserved;
        uint8_t creation_time_precision;
        uint16_t creation_time;
        uint16_t creation_date;
        uint16_t accessed_date;
        uint16_t first_cluster_up;
        uint16_t modification_time;
        uint16_t modification_date;
        uint16_t first_cluster_lo;
        uint32_t size;
} fat_file;

typedef struct fat_long_filename {
        uint8_t order;
        char16 chars0[5];
        uint8_t attribute;
        uint8_t long_entry_type;
        uint8_t checksum;
        char16 chars1[6];
        uint16_t _zero;
        char16 chars2[2];
}__attribute__((packed)) fat_long_filename;

typedef struct fat_volume {
        fat_type type;
        fat_boot_sector *bpb;
} fat_volume;

extern fs_handler fat32_fs_handler;

fs_mountpoint *fat32_mount(gpt_partition *partition, fs_path *path);
int fat32_unmount(fs_mountpoint *mountpoint);
fs_file *fat32_open(fs_mountpoint *, fs_path *path, fs_flags flags);
size_t fat32_read(fs_file *file, void *buffer, size_t size);
size_t fat32_read_file(fs_file *file, void *buffer, size_t size);
size_t fat32_read_dir(fs_file *file, void *buffer, size_t size);
bool fat32_close(fs_file *file);
