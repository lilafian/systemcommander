/* SPDX-License-Identifier: GPL-3.0-or-later
 * Declares functions and structures for interacting with filesystems.
 * Copyright (C) 2026 lilaf */

#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <syscom/gpt.h>

#define FS_TYPE_NONE 0
#define FS_TYPE_FAT32 1
#define FS_TYPE_EXT2 2

#define O_ACCMODE 00000003
#define O_RDONLY 00000000
#define O_WRONLY 00000001
#define O_RDWR 00000002
#define O_CREAT 00000100
#define O_EXCL 00000200
#define O_NOCTTY 00000400
#define O_TRUNC 00001000
#define O_APPEND 00002000
#define O_NONBLOCK 00004000
#define O_DSYNC 00010000
#define FASYNC 00020000
#define O_DIRECT 00040000
#define O_LARGEFILE 00100000
#define O_DIRECTORY 00200000
#define O_NOFOLLOW 00400000
#define O_NOATIME 01000000
#define O_CLOEXEC 02000000
#define __O_SYNC 04000000
#define O_SYNC (__O_SYNC | O_DSYNC)
#define O_PATH 010000000
#define __O_TMPFILE 020000000
#define O_TMPFILE (__O_TMPFILE | O_DIRECTORY)
#define O_NDELAY O_NONBLOCK

#define S_FMT 0170000
#define S_FREG 0100000
#define S_FDIR 0040000
#define S_RUSR 0400
#define S_WUSR 0200
#define S_XUSR 0100

#define FS_MAX_MOUNTED_FILESYSTEMS 256

#define UNMOUNT_ERR_NOT_MOUNTED 1

typedef uint64_t timestamp; // POSIX time

typedef struct fs_path {
        char **components;
        size_t depth;
} fs_path;

typedef uint16_t fs_mode;
typedef uint32_t fs_flags;
typedef uint16_t fs_type;

typedef struct fs_mountpoint {
        fs_path *path;
        gpt_partition *partition;
        struct fs_handler *handler;
        void *driver_data;
} fs_mountpoint;

typedef struct fs_active_mountpoints_info {
        fs_mountpoint **mountpoints;
        int active_mountpoint_count;
} fs_active_mountpoints_info;

typedef struct fs_file {
        fs_mode mode;
        fs_flags flags;
        fs_mountpoint *parent_mount;
        uint32_t seek;
        uint32_t size;
        void *driver_data;
} fs_file;

typedef struct fs_file_info {
        char *name;
        timestamp creation_time;
        timestamp modification_time;
        fs_mode mode;
        uint32_t size;
} fs_file_info;

typedef struct fs_handler {
        fs_type type;
        fs_mountpoint *(*mount)(gpt_partition *, fs_path *);
        int (*unmount)(fs_mountpoint *mountpoint);
        fs_file *(*open)(fs_mountpoint *, fs_path *, fs_flags);
        size_t (*read)(fs_file *, void *, size_t);
        size_t (*write)(fs_file *, void *, size_t);
        fs_file_info *(*stat)(fs_file *);
        bool (*close)(fs_file *);
} fs_handler;

extern fs_path root_path;

fs_path *create_path(char *in, int depth);
void free_path(fs_path *path);
bool paths_equal(fs_path *p1, fs_path *p2);

fs_mountpoint *mount(gpt_partition *partition, fs_path *path, fs_handler *handler);
int unmount(fs_path *path);
fs_active_mountpoints_info *get_mount_points();
fs_mountpoint *resolve_mountpoint(fs_path *path);
fs_path *strip_mountpoint(fs_path *path, fs_mountpoint *mountpoint);

fs_file *fopen(fs_path *path, fs_flags flags);
bool fclose(fs_file *file);
size_t fread(fs_file *file, void *buffer, size_t size); // when file is a dir, will return an array of fs_file_info *s into the buffer
size_t fwrite(fs_file *file, void *inbuf, size_t size);
fs_file_info *fstat(fs_file *file);
