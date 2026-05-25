/* SPDX-License-Identifier: GPL-3.0-or-later
 * Declares structures and functions for working with ext2 filesystems.
 * Copyright (C) 2026 lilaf */

#pragma once
#include <syscom/fs.h>

#define EXT2_STATE_CLEAN 1
#define EXT2_STATE_ERROR 2

#define EXT2_ERR_HANDLING_IGNORE 1
#define EXT2_ERR_HANDLING_REMOUNT_RO 2
#define EXT2_ERR_HANDLING_PANIC 3

#define EXT2_OS_ID_LINUX 0
#define EXT2_OS_ID_HURD 1
#define EXT2_OS_ID_MASIX 2
#define EXT2_OS_ID_FREEBSD 3
#define EXT2_OS_ID_DARWIN_OTHER 4

#define EXT2_OPTIONAL_FEATURE_PREALLOCATE_NEW_DIRS 0x0001
#define EXT2_OPTIONAL_FEATURE_AFS_SERVER_INODES 0x0002
#define EXT2_OPTIONAL_FEATURE_JOURNALING 0x0004
#define EXT2_OPTIONAL_FEATURE_INODE_EXTENDED_ATTRIBUTES 0x0008
#define EXT2_OPTIONAL_FEATURE_RESIZABLE_FS 0x0010
#define EXT2_OPTIONAL_FEATURE_DIR_HASH_INDEX 0x0020

#define EXT2_REQUIRED_FEATURE_COMPRESSION 0x0001
#define EXT2_REQUIRED_FEATURE_DIR_TYPE 0x0002
#define EXT2_REQUIRED_FEATURE_JOURNAL_REPLAY 0x0004
#define EXT2_REQUIRED_FEATURE_JOURNAL_DEVICE 0x0008

#define EXT2_RO_FEATURE_SPARSE_SUPERBLOCKS 0x0001
#define EXT2_RO_FEATURE_64_BIT_FILE_SIZE 0x0002
#define EXT2_RO_FEATURE_BINARY_TREE_DIRS 0x0004

#define EXT2_INODE_TYPE_MASK 0xF000
#define EXT2_INODE_TYPE_FIFO 0x1000
#define EXT2_INODE_TYPE_CHARDEV 0x2000
#define EXT2_INODE_TYPE_DIRECTORY 0x4000
#define EXT2_INODE_TYPE_BLOCKDEV 0x6000
#define EXT2_INODE_TYPE_REGULAR_FILE 0x8000
#define EXT2_INODE_TYPE_SYMLINK 0xA000
#define EXT2_INODE_TYPE_SOCKET 0xC000

#define EXT2_DIRENT_TYPE_UNKNOWN 0
#define EXT2_DIRENT_TYPE_REGULAR_FILE 1
#define EXT2_DIRENT_TYPE_DIRECTORY 2
#define EXT2_DIRENT_TYPE_CHARDEV 3
#define EXT2_DIRENT_TYPE_BLOCKDEV 4
#define EXT2_DIRENT_TYPE_FIFO 5
#define EXT2_DIRENT_TYPE_SOCKET 6
#define EXT2_DIRENT_TYPE_SYMLINK 7

#define EXT2_ROOT_INODE 2

typedef struct ext2_superblock {
        uint32_t total_inodes;
        uint32_t total_blocks;
        uint32_t su_reserved_blocks;
        uint32_t unallocated_blocks;
        uint32_t unallocated_inodes;
        uint32_t superblock_block;
        uint32_t block_size_shift; // 1024 << this = block size
        uint32_t fragment_size_shift; // see above
        uint32_t blocks_per_group;
        uint32_t fragments_per_group;
        uint32_t inodes_per_group;
        uint32_t last_mount_time; // posix time
        uint32_t last_write_time; // see above
        uint16_t mounts_since_fsck;
        uint16_t mounts_before_fsck_required;
        uint16_t signature;
        uint16_t state;
        uint16_t error_handling;
        uint16_t version_minor;
        uint32_t last_fsck_time; // posix time
        uint32_t forced_fsck_interval; // see above
        uint32_t os_id;
        uint32_t version_major;
        uint16_t reserved_user_id;
        uint16_t reserved_group_id;
        uint32_t first_unreserved_inode; // START OF EXTENDED SUPERBLOCK -- only if version_major >= 1
        uint16_t inode_size;
        uint16_t superblock_group;
        uint32_t optional_features;
        uint32_t required_features;
        uint32_t readonly_features;
        char uuid[16];
        char name[16];
        char last_mount_point[64];
        uint32_t compression_used;
        uint16_t _unused;
        char journal_id[16];
        uint32_t journal_inode;
        uint32_t journal_device;
        uint32_t orphan_inode_list_head;
}__attribute__((packed)) ext2_superblock;

typedef struct ext2_block_group_descriptor {
        uint32_t block_usage_bitmap_address;
        uint32_t inode_usage_bitmap_address;
        uint32_t inode_table_address;
        uint16_t unallocated_blocks;
        uint16_t unallocated_inodes;
        uint16_t directories;
        uint8_t _padding[14];
}__attribute__((packed)) ext2_block_group_descriptor;

typedef struct ext2_inode {
        uint16_t type_permissions;
        uint16_t uid_lo;
        uint32_t size_lo;
        uint32_t last_access_time;
        uint32_t creation_time;
        uint32_t last_modification_time;
        uint32_t deletion_time;
        uint16_t gid_lo;
        uint16_t hard_links;
        uint32_t sectors_used;
        uint32_t flags;
        uint32_t _reserved0;
        uint32_t direct_block_pointer[12];
        uint32_t singly_indirect_block_pointer; // Points to a block that is a list of block pointers to data
        uint32_t doubly_indirect_block_pointer; // Points to a block that is a list of block pointers to singly indirect blocks
        uint32_t triply_indirect_block_pointer; // Points to a block that is a list of block pointers to doubly indirect blocks
        uint32_t generation_number;
        uint32_t extended_attribute_block;
        uint32_t size_up_or_dir_acl;
        uint32_t fragment_block_address;
        uint8_t fragment_number;
        uint8_t fragment_size;
        uint16_t _reserved1;
        uint16_t uid_up;
        uint16_t gid_up;
}__attribute__((packed)) ext2_inode;

typedef struct ext2_dirent {
        uint32_t inode;
        uint16_t entry_size;
        uint8_t name_length_lo;
        uint8_t type_indicator_or_name_length_up;
        char name_start;
}__attribute__((packed)) ext2_dirent;

typedef struct ext2_file {
        ext2_inode *inode;
        ext2_dirent *dirent;
}__attribute__((packed)) ext2_file;

typedef struct ext2_volume {
        ext2_superblock *superblock;
        bool read_only;
}__attribute__((packed)) ext2_volume;

extern fs_handler ext2_fs_handler;

fs_mountpoint *ext2_mount(gpt_partition *partition, fs_path *path);
int ext2_unmount(fs_mountpoint *mountpoint);
fs_file *ext2_open(fs_mountpoint *mountpoint, fs_path *path, fs_flags flags);
bool ext2_close(fs_file *file);
size_t ext2_read(fs_file *file, void *buffer, size_t size);
size_t ext2_read_file(fs_file *file, void *buffer, size_t size);
size_t ext2_read_dir(fs_file *file, fs_file_info **buffer, size_t size);
size_t ext2_write(fs_file *file, void *inbuf, size_t size);
fs_file_info *ext2_stat(fs_file *file);
