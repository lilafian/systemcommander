/* SPDX-License-Identifier: GPL-3.0-or-later
 * Defines functions for interacting with filesystems.
 * Copyright (C) 2026 lilaf */

#include <syscom/fs.h>
#include <syscom/string.h>
#include <syscom/stdmemory.h>
#include <syscom/heap.h>

fs_mountpoint *mounted_filesystems[FS_MAX_MOUNTED_FILESYSTEMS];
int mounted_filesystem_count = 0;

fs_path root_path = {
        .components = NULL,
        .depth = 0
};

fs_path *create_path(char *in, int depth) {
        fs_path *path = malloc(sizeof(fs_path));
        path->components = NULL;
        path->depth = 0;

        if (strlen(in) == 1) return path; // Root

        char **components = malloc(sizeof(char *) * depth);
        char *component = malloc(255);
        int components_count = 0;
        int current_length = 0;

        for (int i = 1; in[i] != '\0' && components_count < depth; i++) {
                if (in[i] == '/' || current_length >= 255) {
                        component[current_length] = '\0';
                        components[components_count++] = component;
                        current_length = 0;

                        component = malloc(255);
                        continue;
                }

                component[current_length] = in[i];
                current_length++;
        }

        if (current_length > 0) {
                component[current_length] = '\0';
                components[components_count++] = component;
        }

        path->components = components;
        path->depth = components_count;
        return path;
}

void free_path(fs_path *path) {
        free(path->components);
}

bool paths_equal(fs_path *p1, fs_path *p2) {
        if (p1->depth != p2->depth) return false;
        if (p1->depth == 0 && p2->depth == 0) return true;

        for (int i = 0; i < p1->depth; i++) {
                if (strncmp(p1->components[i], p2->components[i], 255) != 0) return false;
        }
        return true;
}

fs_mountpoint *mount(gpt_partition *partition, fs_path *path, fs_handler *handler) {
        fs_mountpoint *mp = handler->mount(partition, path);
        mounted_filesystems[mounted_filesystem_count] = mp;
        mounted_filesystem_count++;
        return mp;
}

int unmount(fs_path *path) {
        for (int i = 0; i < mounted_filesystem_count; i++) {
                fs_mountpoint *mp = mounted_filesystems[i];
                if (!paths_equal(mp->path, path)) continue;

                int result = mp->handler->unmount(mp);

                for (int j = i; j < mounted_filesystem_count - 1; j++) {
                        mounted_filesystems[j] = mounted_filesystems[j + 1];
                }

                mounted_filesystem_count--;

                return result;
        }

        return UNMOUNT_ERR_NOT_MOUNTED;
}

fs_mountpoint *resolve_mountpoint(fs_path *path) {
        fs_mountpoint *mountpoint = NULL;
        size_t highest_depth = 0;
        for (int i = 0; i < mounted_filesystem_count; i++) {
                fs_mountpoint *mp = mounted_filesystems[i];
                if (mp->path->depth > path->depth) continue;

                bool match = true;
                if (!paths_equal(mp->path, &root_path)) {
                        for (int j = 0; j < mp->path->depth; j++) {
                                if (mp->path->components[j] != path->components[j]) {
                                        match = false;
                                        break;
                                }
                        }
                }
                if (match && mp->path->depth >= highest_depth) {
                        mountpoint = mp;
                        highest_depth = mp->path->depth;
                }
        }

        return mountpoint;
}

// this assumes mountpoint is actually correct
fs_path *strip_mountpoint(fs_path *path, fs_mountpoint *mountpoint) {
        if (!mountpoint || paths_equal(mountpoint->path, &root_path)) return path;

        fs_path *new_path = malloc(sizeof(fs_path));
        char **components = malloc(sizeof(char *) * (path->depth - mountpoint->path->depth));
        new_path->components = components;

        for (int i = 0; i < path->depth - mountpoint->path->depth; i++) {
                new_path->components[i] = path->components[i + mountpoint->path->depth];
        }
        
        new_path->depth = path->depth - mountpoint->path->depth;

        return new_path;
}

fs_active_mountpoints_info *get_mount_points() {
        fs_active_mountpoints_info *info = malloc(sizeof(fs_active_mountpoints_info));
        info->mountpoints = mounted_filesystems;
        info->active_mountpoint_count = mounted_filesystem_count;
        return info;
}

fs_file *fopen(fs_path *path, fs_flags flags) {
        fs_mountpoint *mountpoint = resolve_mountpoint(path);
        if (!mountpoint) return NULL;
        fs_path *new_path = strip_mountpoint(path, mountpoint);
        return mountpoint->handler->open(mountpoint, new_path, flags);
}

size_t fread(fs_file *file, void *buffer, size_t size) { // when file is a dir, will return an array of fs_file_info structs into the buffer
        return file->parent_mount->handler->read(file, buffer, size);
}

bool fclose(fs_file *file) {
        return file->parent_mount->handler->close(file);
}

fs_file_info *fstat(fs_file *file) {
        return file->parent_mount->handler->stat(file);
}
