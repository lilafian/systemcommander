/* SPDX-License-Identifier: GPL-3.0-or-later
 * Defines functions for working with FAT filesystems.
 * Copyright (C) 2026 lilaf */

#include <syscom/fs/fat.h>
#include <syscom/gpt.h>
#include <syscom/string.h>
#include <syscom/heap.h>
#include <syscom/log.h>

fs_handler fat32_fs_handler = {
        .type = FS_TYPE_FAT32,
        .mount = fat32_mount,
        .unmount = fat32_unmount,
        .open = fat32_open,
        .read = fat32_read,
        .close = fat32_close
};

fs_mountpoint *fat32_mount(gpt_partition *partition, fs_path *path) {
        fs_mountpoint *mountpoint = malloc(sizeof(fs_mountpoint));
        mountpoint->path = path;
        mountpoint->handler = &fat32_fs_handler;
        mountpoint->partition = partition;

        fat_boot_sector *boot_sector = malloc(512);
        bool read_success = gpt_read_partition(partition->ahci, partition->entry, 0, 1, boot_sector);
        if (!read_success) {
                free(boot_sector);
                free(mountpoint);
                return NULL;
        }

        fat_volume *volume = malloc(sizeof(fat_volume));
        volume->type = FAT_TYPE_FAT32;
        volume->bpb = boot_sector;

        mountpoint->driver_data = volume;

        return mountpoint;
}

int fat32_unmount(fs_mountpoint *mountpoint) {
        free(((fat_volume *)(mountpoint->driver_data))->bpb);
        free(mountpoint->driver_data);
        free(mountpoint);
        return 0;
}

// todo: make this actually read the right encoding
void fat_read_lfn_entry(fat_long_filename *entry, char *buf) {
        char *temp = malloc(13);
        memset(temp, 0, 13);
        for (int i = 0; i < 5; i++) {
                char16 c = entry->chars0[i];
                if (c == 0xFFFF) continue;
                strchrcat(temp, (char)(c));
        }

        for (int i = 0; i < 6; i++) {
                char16 c = entry->chars1[i];
                if (c == 0xFFFF) continue;
                strchrcat(temp, (char)(c));
        }

        for (int i = 0; i < 2; i++) {
                char16 c = entry->chars2[i];
                if (c == 0xFFFF) continue;
                strchrcat(temp, (char)(c));
        }

        memmove(buf + 13, buf, 243);
        memcpy(buf, temp, 13);
        free(temp);
}

uint32_t fat32_next_cluster(fs_mountpoint *mp, uint32_t cluster) {
        uint8_t fat_table[512];
        uint32_t fat_offset = cluster * 4;
        uint32_t fat_sector = ((fat_volume *)mp->driver_data)->bpb->reserved_sectors + (fat_offset / 512);
        uint32_t entry_offset = fat_offset % 512;

        gpt_read_partition(mp->partition->ahci, mp->partition->entry, fat_sector, 1, fat_table);

        return (*(uint32_t *)&fat_table[entry_offset]) & 0x0FFFFFFF;
}
        
fs_file *fat_open_fat_file(fat_file *file, fs_flags flags, fs_mountpoint *parent_mount) {
        fs_file *fsfile = malloc(sizeof(fs_file));
        memset(fsfile, 0, sizeof(fs_file));

        fsfile->mode |= (file->attributes & FAT_FILE_ATTRIB_DIRECTORY ? S_FDIR : 0);
        fsfile->mode |= (file->attributes & FAT_FILE_ATTRIB_HIDDEN ? S_FHDN : 0);
        fsfile->mode |= (file->attributes & FAT_FILE_ATTRIB_SYSTEM ? S_FHDN : 0);
        fsfile->mode |= S_XUSR; // FAT32 has no executable attribute, so for now just allow all files to be executed
        fsfile->mode |= S_RUSR;
        fsfile->mode |= S_WUSR;
        if (file->attributes & FAT_FILE_ATTRIB_READ_ONLY) fsfile->mode &= ~S_WUSR;

        fsfile->flags = flags;
        fsfile->parent_mount = parent_mount;
        fsfile->seek = 0;
        fsfile->size = file->size;
        fsfile->driver_data = file;

        return fsfile;
}

fat_file *fat32_search_dir(fs_mountpoint *mountpoint, uint32_t cluster, char *filename) {
        fat_volume *volume = mountpoint->driver_data;
        fat32_extended_boot_sector ext = *(fat32_extended_boot_sector*)volume->bpb->extended;

        while (cluster < 0x0FFFFFF7) {
                uint32_t fat_size = volume->bpb->sectors_per_fat == 0 ? ext.sectors_per_fat : volume->bpb->sectors_per_fat;
                uint32_t first_data_sector = volume->bpb->reserved_sectors + (volume->bpb->fats * fat_size);
                uint32_t first_sector = ((cluster - 2) * volume->bpb->sectors_per_cluster) + first_data_sector;
                char *read_buf = malloc(volume->bpb->sectors_per_cluster * 512);
                bool read_success = gpt_read_partition(mountpoint->partition->ahci, mountpoint->partition->entry, first_sector, volume->bpb->sectors_per_cluster, read_buf);
                if (!read_success) return NULL;

                uint8_t *entry = read_buf;
                char *lfn_entry = malloc(256);
                memset(lfn_entry, 0, 256);
                while (entry[0] != 0) {
                        if (entry[0] == 0xe5) {
                                entry += 32;
                                continue;
                        }

                        if (entry[11] == 0x0f) {
                                fat_read_lfn_entry((fat_long_filename *)entry, lfn_entry);
                                entry += 32;
                                continue;
                        }

                        fat_file *file = (fat_file *)entry;
                        char *name = NULL;
                        uint16_t name_max_size = 0;
                        if (lfn_entry[0] != '\0') {
                                name_max_size = 256;
                                name = malloc(256);
                                memcpy(name, lfn_entry, 256);
                        } else {
                                name_max_size = 13;
                                name = malloc(13);
                                memset(name, 0, 13);
                                for (int i = 0; i < 13; i++) {
                                        if (file->name[i] == ' ' && (i == 13 || file->name[i + 1] == ' ')) continue;
                                        name[i] = file->name[i];
                                }
                                if (file->extension[0] != ' ' && file->extension[0] != '\0') {
                                        strchrcat(name, '.');
                                        strcat(name, file->extension);
                                }
                        }

                        memset(lfn_entry, 0, 256);

                        if (strncmp(name, filename, name_max_size) == 0 && !(file->attributes & FAT_FILE_ATTRIB_VOLUME_ID)) {
                                fat_file *result = malloc(sizeof(fat_file));
                                memcpy(result, entry, sizeof(fat_file));
                                free(read_buf);
                                return result;
                        }

                        entry += 32;
                        free(name);
                }
                cluster = fat32_next_cluster(mountpoint, cluster);

                free(lfn_entry);
        }

        return NULL;
}

timestamp fat_time_to_timestamp(uint16_t date, uint16_t time, uint8_t precision) {
        uint64_t year = FAT_DATE_YEAR(date);
        uint64_t month = FAT_DATE_MONTH(date);
        uint64_t day = FAT_DATE_DAY(date);
        uint64_t hour = FAT_TIME_HOUR(time);
        uint64_t min = FAT_TIME_MIN(time);
        uint64_t sec = FAT_TIME_SEC(time);

        uint64_t leaps = year/4 - year/100 + year/400;
        uint64_t leaps0 = 1969/4 - 1969/100 + 1969/400;
        uint64_t leap_days = leaps - leaps0;

        uint64_t days_years = 365 * (year - 1970) + leap_days;

        bool current_is_leap = (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
        uint8_t month_days[13] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
        if (current_is_leap) month_days[2] = 29;

        uint64_t days_months = 0;
        for (int i = 1; i < month; i++) {
                days_months += month_days[i];
        }

        uint64_t total_days = days_years + days_months + (day - 1);
        return ((total_days * 86400) + (hour * 3600) + (min * 60) + sec) * 100 + precision;
}

fs_file *fat32_open(fs_mountpoint *mountpoint, fs_path *path, fs_flags flags) {
        fat_volume *volume = mountpoint->driver_data;
        fat32_extended_boot_sector ext = *(fat32_extended_boot_sector*)volume->bpb->extended;
        uint32_t root_cluster = ext.root_cluster;
        uint32_t cluster = root_cluster;

        fat_file *file = NULL;
        for (size_t i = 0; i < path->depth; i++) {
                file = fat32_search_dir(mountpoint, cluster, path->components[i]);
                if (!file) {
                        logf("[fs:fat32_open] <FATAL> File %s does not exist! (Component %d/%d of path)\n", path->components[i], i + 1, path->depth);
                        return NULL;
                }
                cluster = (uint32_t)file->first_cluster_lo + ((uint32_t)file->first_cluster_up << 16);
        }

        if (path->depth == 0) {
                file = malloc(sizeof(fat_file));
                memset(file, 0, sizeof(fat_file));
                file->attributes = FAT_FILE_ATTRIB_DIRECTORY;
                file->first_cluster_lo = (uint16_t)root_cluster;
                file->first_cluster_up = (uint16_t)(root_cluster >> 16);
        }

        if (!file) {
                log("[fs:fat32_open] <FATAL> Failed to open file\n");
                return NULL;
        }

        return fat_open_fat_file(file, flags, mountpoint);
}

size_t fat32_read(fs_file *file, void *buffer, size_t size) {
        if (file->mode & S_FDIR) {
                return fat32_read_dir(file, (fs_file_info **)buffer, size);
        }
        return fat32_read_file(file, buffer, size);
}

size_t fat32_read_file(fs_file *file, void *buffer, size_t size) {
        fs_mountpoint *mountpoint = file->parent_mount;
        fat_volume *volume = mountpoint->driver_data;
        fat32_extended_boot_sector ext = *(fat32_extended_boot_sector*)volume->bpb->extended;

        if (size > file->size) size = file->size;
        
        uint16_t cluster_lo = ((fat_file *)file->driver_data)->first_cluster_lo;
        uint16_t cluster_up = ((fat_file *)file->driver_data)->first_cluster_up;
        uint32_t cluster = (uint32_t)cluster_lo + ((uint32_t)cluster_up << 16);
        
        size_t written = 0;

        while (cluster < 0x0FFFFFF7 && written < size) {
                uint32_t fat_size = volume->bpb->sectors_per_fat == 0 ? ext.sectors_per_fat : volume->bpb->sectors_per_fat;
                uint32_t first_data_sector = volume->bpb->reserved_sectors + (volume->bpb->fats * fat_size);
                uint32_t first_sector = ((cluster - 2) * volume->bpb->sectors_per_cluster) + first_data_sector;
                size_t bytes_per_cluster = volume->bpb->sectors_per_cluster * 512;
                char *read_buf = malloc(bytes_per_cluster);
                bool read_success = gpt_read_partition(mountpoint->partition->ahci, mountpoint->partition->entry, first_sector, volume->bpb->sectors_per_cluster, read_buf);
                if (!read_success) return 0;

                size_t read_size = (size - written < bytes_per_cluster) ? size - written : bytes_per_cluster;

                memcpy(buffer + written, read_buf, read_size);
                written += read_size;

                cluster = fat32_next_cluster(mountpoint, cluster);
        }

        return written;
}

size_t fat32_read_dir(fs_file *file, fs_file_info **buffer, size_t size) {
        fs_mountpoint *mountpoint = file->parent_mount;
        fat_volume *volume = mountpoint->driver_data;
        fat32_extended_boot_sector ext = *(fat32_extended_boot_sector*)volume->bpb->extended;

        uint16_t cluster_lo = ((fat_file *)file->driver_data)->first_cluster_lo;
        uint16_t cluster_up = ((fat_file *)file->driver_data)->first_cluster_up;
        uint32_t cluster = (uint32_t)cluster_lo + ((uint32_t)cluster_up << 16);

        size_t max_entries = size / sizeof(fs_file_info *);
        size_t entries_written = 0;

        while (cluster < 0x0FFFFFF7 && entries_written < max_entries) {
                uint32_t fat_size = volume->bpb->sectors_per_fat == 0 ? ext.sectors_per_fat : volume->bpb->sectors_per_fat;
                uint32_t first_data_sector = volume->bpb->reserved_sectors + (volume->bpb->fats * fat_size);
                uint32_t first_sector = ((cluster - 2) * volume->bpb->sectors_per_cluster) + first_data_sector;
                char *read_buf = malloc(volume->bpb->sectors_per_cluster * 512);
                bool read_success = gpt_read_partition(mountpoint->partition->ahci, mountpoint->partition->entry, first_sector, volume->bpb->sectors_per_cluster, read_buf);
                if (!read_success) return 0;

                uint8_t *entry = read_buf;
                char *lfn_entry = malloc(256);
                memset(lfn_entry, 0, 256);
                while (entry[0] != 0) {
                        if (entry[0] == 0xe5) {
                                entry += 32;
                                continue;
                        }

                        if (entry[11] == 0x0f) {
                                fat_read_lfn_entry((fat_long_filename *)entry, lfn_entry);
                                entry += 32;
                                continue;
                        }

                        fat_file *file = (fat_file *)entry;
                        char *name = NULL;
                        uint16_t name_max_size = 0;
                        if (lfn_entry[0] != '\0') {
                                name_max_size = 256;
                                name = malloc(256);
                                memcpy(name, lfn_entry, 256);
                        } else {
                                name_max_size = 13;
                                name = malloc(13);
                                memset(name, 0, 13);
                                for (int i = 0; i < 13; i++) {
                                        if (file->name[i] == ' ' && (i == 13 || file->name[i + 1] == ' ')) continue;
                                        name[i] = file->name[i];
                                }
                                if (file->extension[0] != ' ' && file->extension[0] != '\0') {
                                        strchrcat(name, '.');
                                        strcat(name, file->extension);
                                }
                        }

                        fs_file_info *info = malloc(sizeof(fs_file_info));

                        info->name = malloc(256);
                        memcpy(info->name, name, 256);

                        info->mode |= (file->attributes & FAT_FILE_ATTRIB_DIRECTORY ? S_FDIR : 0);
                        info->mode |= (file->attributes & FAT_FILE_ATTRIB_HIDDEN ? S_FHDN : 0);
                        info->mode |= (file->attributes & FAT_FILE_ATTRIB_SYSTEM ? S_FHDN : 0);
                        info->mode |= S_XUSR; // FAT32 has no executable attribute, so for now just allow all files to be executed
                        info->mode |= S_RUSR;
                        info->mode |= S_WUSR;
                        if (file->attributes & FAT_FILE_ATTRIB_READ_ONLY) info->mode &= ~S_WUSR;

                        info->creation_time = fat_time_to_timestamp(file->creation_date, file->creation_time, file->creation_time_precision);
                        info->modification_time = fat_time_to_timestamp(file->modification_date, file->modification_time, 0);

                        info->size = file->size;

                        buffer[entries_written] = info;
                        entries_written++;

                        memset(lfn_entry, 0, 256);
                        entry += 32;
                        free(name);
                }
                cluster = fat32_next_cluster(mountpoint, cluster);

                free(lfn_entry);
        }

        return entries_written * sizeof(fs_file_info *);
}

bool fat32_close(fs_file *file) {

}
