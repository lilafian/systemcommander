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
                                strcat(name, file->name);
                                if (file->extension[0] != '\0') {
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

        if (!file) {
                        log("[fs:fat32_open] <FATAL> Failed to open file");
        }

        if (path->depth == 0) {
                file = malloc(sizeof(fat_file));
                memset(file, 0, sizeof(fat_file));
                file->attributes = FAT_FILE_ATTRIB_DIRECTORY;
                file->first_cluster_lo = (uint16_t)root_cluster;
                file->first_cluster_up = (uint16_t)(root_cluster >> 16);
        }

        return fat_open_fat_file(file, flags, mountpoint);
}

size_t fat32_read(fs_file *file, void *buffer, size_t size) {
        if (file->mode & S_FDIR) {
                return fat32_read_dir(file, buffer, size);
        }
        return fat32_read_file(file, buffer, size);
}

size_t fat32_read_file(fs_file *file, void *buffer, size_t size) {

}

size_t fat32_read_dir(fs_file *file, void *buffer, size_t size) {

}

bool fat32_close(fs_file *file) {

}
