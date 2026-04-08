/* SPDX-License-Identifier: GPL-3.0-or-later
 * Declares functions and structures for interacting with filesystems.
 * Copyright (C) 2026 lilaf */

/* This is a draft! 
 * Nothing here is likely to be in the exact same form */

#pragma once

#define FS_PATH_DEFAULT_MAX_DEPTH 255

#define FS_TYPE_NONE 0
#define FS_TYPE_FAT32 1

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

typedef char **fs_path;
typedef uint8_t fs_mode;
typedef uint16_t fs_type;

typedef struct fs_mountpoint {
        fs_path path;
        struct fs_handler handler;
} fs_mountpoint;

typedef struct fs_file {
        fs_path path;
        fs_path relative_path;
        fs_mode mode;
        fs_mountpoint parent_mount;
} fs_file;

typedef struct fs_handler {
        fs_type type;
        bool (*open)(fs_path);
        bool (*read)(fs_file);
        bool (*close)(fs_file);
} fs_handler;

void create_path(char *in, fs_path *out, int max_depth);
