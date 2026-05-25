/* SPDX-License-Identifier: GPL-3.0-or-later
 * Defines functions for working with ext2 filesystems.
 * Copyright (C) 2026 lilaf */

#include <syscom/fs.h>
#include <syscom/fs/ext2.h>
#include <syscom/panic.h>
#include <syscom/log.h>
#include <syscom/heap.h>
#include <syscom/string.h>

fs_handler ext2_fs_handler = {
        .type = FS_TYPE_EXT2,
        .mount = ext2_mount,
        .unmount = ext2_unmount,
        .open = ext2_open,
        .close = ext2_close,
        .read = ext2_read,
        .write = ext2_write,
        .stat = ext2_stat
};

fs_mountpoint *ext2_mount(gpt_partition *partition, fs_path *path) {
        fs_mountpoint *mountpoint = malloc(sizeof(fs_mountpoint));
        mountpoint->path = path;
        mountpoint->handler = &ext2_fs_handler;
        mountpoint->partition = partition;

        ext2_superblock *superblock = malloc(1024);
        bool read_success = gpt_read_partition(partition->ahci, partition->entry, 2, 2, superblock);
        if (!read_success) {
                free(superblock);
                free(mountpoint);
                return NULL;
        }

        bool read_only = false;
        if (superblock->state == EXT2_STATE_ERROR) {
                switch(superblock->error_handling) {
                        case EXT2_ERR_HANDLING_REMOUNT_RO:
                                read_only = true;
                                break;
                        case EXT2_ERR_HANDLING_PANIC:
                                panic("[fs:ext2_mount] ext2 filesystem has errors and requires a panic on error");
                                break;
                        default:
                                break;
                }
        }

        ext2_volume *volume = malloc(sizeof(ext2_volume));
        volume->superblock = superblock;
        volume->read_only = read_only;

        mountpoint->driver_data = volume;

        return mountpoint;
}

int ext2_unmount(fs_mountpoint *mountpoint) {
        free(((ext2_volume *)(mountpoint->driver_data))->superblock);
        free(mountpoint->driver_data);
        free(mountpoint);
        return 0;
}

ext2_inode *ext2_get_inode(fs_mountpoint *mountpoint, uint32_t inode) {
        ext2_superblock *superblock = ((ext2_volume *)(mountpoint->driver_data))->superblock;
        gpt_partition *partition = mountpoint->partition;
        uint32_t block_size = 1024 << superblock->block_size_shift;
        uint32_t blocks_per_group = superblock->blocks_per_group;
        uint32_t inodes_per_group = superblock->inodes_per_group;
        uint32_t bgdt_start_block = (block_size == 1024) ? 2 : 1;
        uint32_t inode_block_group = (inode - 1) / inodes_per_group;
        uint32_t sectors_per_block = block_size / 512;
        uint32_t bgd_offset_bytes = inode_block_group * sizeof(ext2_block_group_descriptor);
        uint32_t bgd_block = bgdt_start_block + (bgd_offset_bytes / block_size);
        uint32_t bgd_index_in_block = inode_block_group % (block_size / sizeof(ext2_block_group_descriptor));

        ext2_block_group_descriptor *bgdt = malloc(block_size);
        uint8_t *buf = malloc(block_size);
        gpt_read_partition(partition->ahci, partition->entry, bgd_block * sectors_per_block, sectors_per_block, bgdt);
        ext2_block_group_descriptor *group_descriptor = &bgdt[bgd_index_in_block];

        uint32_t inode_table_block = group_descriptor->inode_table_address;
        uint32_t inode_index = (inode - 1) % inodes_per_group;
        uint16_t inode_size = (superblock->version_major >= 1) ? superblock->inode_size : 128;
        uint32_t block = (inode_index * inode_size) / block_size;
        uint32_t offset = (inode_index * inode_size) % block_size;
        uint32_t abs_block = inode_table_block + block;
        gpt_read_partition(partition->ahci, partition->entry, abs_block * sectors_per_block, sectors_per_block, buf);

        ext2_inode *full_inode = malloc(sizeof(ext2_inode));
        memcpy(full_inode, buf + offset, sizeof(ext2_inode));
        free(buf);
        free(bgdt);
        return full_inode;
}

size_t ext2_get_inode_data(fs_mountpoint *mountpoint, ext2_inode *inode, void *buffer) {
        ext2_superblock *superblock = ((ext2_volume *)(mountpoint->driver_data))->superblock;
        gpt_partition *partition = mountpoint->partition;
        uint32_t block_size = 1024 << superblock->block_size_shift;
        uint32_t sectors_per_block = block_size / 512;
        uint64_t remaining_blocks = inode->sectors_used / sectors_per_block;
        uint64_t i = 0;

        for (i = 0; i < 12; i++) {
                uint32_t block = inode->direct_block_pointer[i];
                if (block == 0) {
                        remaining_blocks = 0;
                        break;
                }
                gpt_read_partition(partition->ahci, partition->entry, block * sectors_per_block, sectors_per_block, (void*)((uint64_t)buffer + (i * block_size)));
                remaining_blocks--;
        }

        if (remaining_blocks > 0) {
                uint32_t indirect_block = inode->singly_indirect_block_pointer;
                uint8_t *indirect_block_content = malloc(block_size);
                gpt_read_partition(partition->ahci, partition->entry, indirect_block * sectors_per_block, sectors_per_block, indirect_block_content);
                uint64_t j = 0;
                while (remaining_blocks > 0) {
                        uint32_t block = ((uint32_t*)indirect_block_content)[j];
                        if (block == 0) break;
                        gpt_read_partition(partition->ahci, partition->entry, block * sectors_per_block, sectors_per_block, (void*)((uint64_t)buffer + (i * block_size)));
                        i++;
                        j++;
                        remaining_blocks--;
                }
                free(indirect_block_content);
        }

        if (remaining_blocks > 0) {
                uint32_t doubly_indirect_block = inode->doubly_indirect_block_pointer;
                uint8_t *doubly_indirect_content = malloc(block_size);
                gpt_read_partition(partition->ahci, partition->entry, doubly_indirect_block * sectors_per_block, sectors_per_block, doubly_indirect_content);
                uint64_t j = 0;
                while (remaining_blocks > 0) {
                        uint32_t indirect_block = ((uint32_t*)doubly_indirect_content)[j];
                        if (indirect_block == 0) break;
                        uint8_t *indirect_block_content = malloc(block_size);
                        gpt_read_partition(partition->ahci, partition->entry, indirect_block * sectors_per_block, sectors_per_block, indirect_block_content);
                        uint64_t k = 0;
                        while (remaining_blocks > 0) {
                                uint32_t block = ((uint32_t*)indirect_block_content)[k];
                                if (block == 0) break;
                                gpt_read_partition(partition->ahci, partition->entry, block * sectors_per_block, sectors_per_block, (void*)((uint64_t)buffer + (i * block_size)));
                                i++;
                                k++;
                                remaining_blocks--;
                        }
                        free(indirect_block_content);
                        j++;
                }
                free(doubly_indirect_content);
        }

        if (remaining_blocks > 0) {
                uint32_t triply_indirect_block = inode->triply_indirect_block_pointer;
                uint8_t *triply_indirect_content = malloc(block_size);
                gpt_read_partition(partition->ahci, partition->entry, triply_indirect_block * sectors_per_block, sectors_per_block, triply_indirect_content);
                uint64_t j = 0;
                while (remaining_blocks > 0) {
                        uint32_t doubly_indirect_block = ((uint32_t*)triply_indirect_content)[j];
                        if (doubly_indirect_block == 0) break;
                        uint8_t *doubly_indirect_content = malloc(block_size);
                        gpt_read_partition(partition->ahci, partition->entry, doubly_indirect_block * sectors_per_block, sectors_per_block, doubly_indirect_content);
                        uint64_t k = 0;
                        while (remaining_blocks > 0) {
                                uint32_t indirect_block = ((uint32_t*)doubly_indirect_content)[k];
                                if (indirect_block == 0) break;
                                uint8_t *indirect_block_content = malloc(block_size);
                                gpt_read_partition(partition->ahci, partition->entry, indirect_block * sectors_per_block, sectors_per_block, indirect_block_content);
                                uint64_t l = 0;
                                while (remaining_blocks > 0) {
                                        uint32_t block = ((uint32_t*)indirect_block_content)[l];
                                        if (block == 0) break;
                                        gpt_read_partition(partition->ahci, partition->entry, block * sectors_per_block, sectors_per_block, (void*)((uint64_t)buffer + (i * block_size)));
                                        i++;
                                        l++;
                                        remaining_blocks--;
                                }
                                free(indirect_block_content);
                                k++;
                        }
                        free(doubly_indirect_content);
                        j++;
                }
                free(triply_indirect_content);
        }

        return (inode->sectors_used / sectors_per_block) * block_size;
}

ext2_file *ext2_search_dir(fs_mountpoint *mountpoint, ext2_inode *inode, char *filename) {
        if ((inode->type_permissions & EXT2_INODE_TYPE_MASK) != EXT2_INODE_TYPE_DIRECTORY) {
                logf("[fs:ext2_search_dir] <FATAL> Attempted to search a non-directory for file %s\n", filename);
                return NULL;
        };

        uint8_t *content = malloc(inode->sectors_used * 512);
        ext2_get_inode_data(mountpoint, inode, content);

        uint64_t i = 0;
        while (i < inode->sectors_used * 512) {
                ext2_dirent *dirent = (ext2_dirent *)((uint64_t)content + i);
                if (dirent->inode == 0) break;
                char *name = &dirent->name_start;
                if (strncmp(name, filename, dirent->name_length_lo) == 0) {
                        free(content);
                        ext2_file *drvfile = malloc(sizeof(ext2_file));
                        drvfile->inode = ext2_get_inode(mountpoint, dirent->inode);
                        drvfile->dirent = dirent;
                        return drvfile;
                }

                i += dirent->entry_size;
        }

        free(content);
        return NULL;
}

fs_file *ext2_open_inode(ext2_inode *inode, fs_flags flags, fs_mountpoint *parent_mount) {
        fs_file *fsfile = malloc(sizeof(fs_file));
        memset(fsfile, 0, sizeof(fs_file));

        fsfile->mode = inode->type_permissions;

        fsfile->flags = flags;
        fsfile->parent_mount = parent_mount;
        fsfile->seek = 0;
        fsfile->size = inode->size_lo;
        
        ext2_file *drvfile = malloc(sizeof(ext2_file));
        drvfile->inode = inode;

        fsfile->driver_data = drvfile;
        return fsfile;
}

fs_file *ext2_open(fs_mountpoint *mountpoint, fs_path *path, fs_flags flags) {
        ext2_inode *inode = ext2_get_inode(mountpoint, EXT2_ROOT_INODE);
        for (size_t i = 0; i < path->depth; i++) {
                ext2_inode *new_inode = ext2_search_dir(mountpoint, inode, path->components[i]);
                free(inode);
                inode = new_inode;
                if (!inode) {
                        logf("[fs:ext2_open] <FATAL> File %s does not exist or read failed for another reason (Component %d/%d of path)\n", path->components[i], i + 1, path->depth);
                        return NULL;
                }
        }

        if (!inode) {
                log("[fs:ext2_open] <FATAL> Failed to open file\n");
                return NULL;
        }

        return ext2_open_inode(inode, flags, mountpoint);
}

bool ext2_close(fs_file *file) {
        free(file->driver_data->inode);
        free(file->driver_data->dirent);
        free(file->driver_data);
        free(file);
        return true;
}

size_t ext2_read(fs_file *file, void *buffer, size_t size) {
        if (file->mode & S_FDIR) {
                return ext2_read_dir(file, (fs_file_info **)buffer, size);
        }
        return ext2_read_file(file, buffer, size);
}

size_t ext2_read_file(fs_file *file, void *buffer, size_t size) {
        ext2_inode *inode = ((ext2_file *)file->driver_data)->inode;
        uint8_t *content = malloc(inode->sectors_used * 512);
        ext2_get_inode_data(file->parent_mount, inode, content);
        uint32_t read_size = (size > file->size - file->seek) ? file->size - file->seek : size;
        memcpy(buffer, content + file->seek, read_size);
        file->seek += read_size;
        free(content);
        return read_size;
}

size_t ext2_read_dir(fs_file *file, fs_file_info **buffer, size_t size) {
        ext2_inode *inode = ((ext2_file *)file->driver_data)->inode;
        if ((inode->type_permissions & EXT2_INODE_TYPE_MASK) != EXT2_INODE_TYPE_DIRECTORY) {
                logf("[fs:ext2_read_dir] <FATAL> Attempted to call ext2_read_dir on a non-directory somehow\n");
                return 0;
        };

        uint8_t *content = malloc(inode->sectors_used * 512);
        ext2_get_inode_data(file->parent_mount, inode, content);

        size_t max_entries = size / sizeof(fs_file_info *);
        size_t entries_written = 0;

        uint64_t i = 0;
        while (i < inode->sectors_used * 512) {
                if (entries_written >= max_entries) break;
                ext2_dirent *dirent = (ext2_dirent *)((uint64_t)content + i);
                if (dirent->inode == 0) break;
                ext2_inode *new_inode = ext2_get_inode(file->parent_mount, dirent->inode);
                char *name = &dirent->name_start;

                fs_file_info *info = malloc(sizeof(fs_file_info));
                info->name = malloc(256);
                memset(info->name, 0, 256);
                memcpy(info->name, name, dirent->name_length_lo);

                info->mode = new_inode->type_permissions;
                info->creation_time = new_inode->creation_time;
                info->modification_time = new_inode->last_modification_time;
                info->size = new_inode->size_lo;

                free(new_inode);

                buffer[entries_written] = info;
                entries_written++;

                if (dirent->entry_size == 0) break;
                i += dirent->entry_size;
        }

        free(content);
        return entries_written * sizeof(fs_file_info *);
}

// TODO: add ext2 write

fs_file_info *ext2_stat(fs_file *file) {
        fs_file_info *info = malloc(sizeof(fs_file_info));
        ext2_inode *inode = ((ext2_file *)file->driver_data)->inode;
        ext2_dirent *dirent = ((ext2_file *)file->driver_data)->dirent;
        memset(info, 0, sizeof(fs_file_info));
 
        info->name = malloc(dirent->name_length_lo + 1);
        memset(info, 0, dirent->name_length_lo + 1);
        memcpy(info->name, dirent->name_start, dirent->name_length_lo);

        info->creation_time = inode->creation_time;
        info->modification_time = inode->last_modification_time;

        info->mode = file->mode;
        info->size = file->size;
        return info;
}
