/* Arduino SdFat Library
 * Copyright (C) 2009 by William Greiman
 *
 * This file is part of the Arduino SdFat Library
 *
 * This Library is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * This Library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * the Arduino SdFat Library.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "sd.h"

#define RETURN_ERR_FILE do { SdFile err; sd_file_init(&err); return err; }while(0)


bool sd_begin(SdClass* sd, SdFileDateTime date_time_callback)
{
    sd_date_time_callback = date_time_callback;

    return sd_card_init(&(sd->card), SPI_HALF_SPEED)
        && sd_volume_init_try_both(&(sd->volume), &(sd->card))
        && sd_file_open_root(&(sd->root), &(sd->volume));
}


SdFile sd_open(SdClass* sd, const char* filepath, uint8_t mode)
{
    int pathidx;

    // do the interative search
    SdFile parentdir = sd_get_parent_dir(sd, filepath, &pathidx);
    // no more subdirs!

    filepath += pathidx;

    if (!filepath[0]) {
        return parentdir; // it was the directory itself!
    }

    // Open the file itself
    SdFile file;
    sd_file_init(&file);

    // failed to open a subdir!
    if (!sd_file_is_open(&parentdir)) {
        RETURN_ERR_FILE;
    }

    // there is a special case for the Root directory since its a static dir
    if (sd_file_is_root(&parentdir)) {
        if (!sd_file_open(&file, &sd->root, filepath, mode)) {
            // failed to open the file :(
            RETURN_ERR_FILE;
        }
        // dont close the root!
    } else {
        if (!sd_file_open(&file, &parentdir, filepath, mode)) {
            RETURN_ERR_FILE;
        }
        // close the parent
        sd_file_close(&parentdir);
    }

    if (mode & (O_APPEND | O_WRITE)) {
        sd_file_seek_set(&file, file.file_size);
    }
    return file;
}



SdFile sd_get_parent_dir(SdClass* sd, const char* filepath, int* index)
{
    // get parent directory
    SdFile d1 = sd->root; // start with the mostparent, root!
    SdFile d2;
    sd_file_init(&d2);

    // we'll use the pointers to swap between the two objects
    SdFile* parent = &d1;
    SdFile* subdir = &d2;

    const char* origpath = filepath;

    while (strchr(filepath, '/')) {
        // strip leading /'s
        if (filepath[0] == '/') {
            filepath++;
            continue;
        }

        if (!strchr(filepath, '/')) {
            break; // it was in the root directory, so leave now
        }

        // extract just the name of the next subdirectory
        uint8_t idx = strchr(filepath, '/') - filepath;
        if (idx > 12) {
            idx = 12;    // dont let them specify long names
        }
        char subdirname[13];
        strncpy(subdirname, filepath, idx);
        subdirname[idx] = 0;

        // close the subdir (we reuse them) if open
        sd_file_close(subdir);

        if (!sd_file_open(subdir, parent, subdirname, O_READ)) {
            return (SdFile) { 0 }; // failed to open one of the subdirectories
        }
        // move forward to the next subdirectory
        filepath += idx;

        // we reuse the objects, close it.
        sd_file_close(parent);

        // swap the pointers
        SdFile* t = parent;
        parent = subdir;
        subdir = t;
    }

    *index = (int) (filepath - origpath);
    // parent is now the parent diretory of the file!
    return *parent;
}


void sd_end(SdClass* sd)
{
    sd_file_close(&sd->root);
}


/*
 * Parse individual path components from a path.
 *
 * e.g. after repeated calls '/foo/bar/baz' will be split into 'foo', 'bar',
 * 'baz'.
 *
 * This is similar to `strtok()` but copies the component into the supplied
 * buffer rather than modifying the original string.
 *
 * `buffer` needs to be PATH_COMPONENT_BUFFER_LEN in size.
 *
 * `p_offset` needs to point to an integer of the offset at which the previous
 * path component finished.
 *
 * Returns `true` if more components remain.
 *
 * Returns `false` if this is the last component. (This means path ended with
 * 'foo' or 'foo/'.)
 */
static bool get_next_path_component(const char* path, unsigned int* p_offset,
                                    char* buffer)
{
    // TODO: Have buffer local to this function, so we know it's the
    //       correct length?

    int buffer_offset = 0;
    int offset = *p_offset;

    // Skip root or other separator
    if (path[offset] == '/') {
        offset++;
    }

    // Copy the next next path segment
    while (buffer_offset < MAX_COMPONENT_LEN
            && path[offset] != '/' && path[offset] != '\0') {
        buffer[buffer_offset++] = path[offset++];
    }

    buffer[buffer_offset] = '\0';

    // Skip trailing separator so we can determine if this is the last
    // component in the path or not.
    if (path[offset] == '/') {
        offset++;
    }

    *p_offset = offset;

    return path[offset] != '\0';
}


bool sd_callback_path_exists(SdFile* parent_dir,
                             const char* file_path_component,
                             __attribute__((unused)) bool is_last_component,
                             __attribute__((unused)) void* object)
{
    SdFile child;
    sd_file_init(&child);

    bool exists = sd_file_open(&child, parent_dir, file_path_component, O_RDONLY);

    if (exists) {
        sd_file_close(&child);
    }

    return exists;
}


bool sd_callback_make_dir_path(SdFile* parent_dir,
                               const char* file_path_component,
                               bool is_last_component, void* object)
{
    bool result = false;
    SdFile child;
    sd_file_init(&child);

    result = sd_callback_path_exists(parent_dir, file_path_component,
                                     is_last_component, object);
    if (!result) {
        result = sd_file_make_dir(&child, parent_dir, file_path_component);
    }

    return result;
}


bool sd_callback_remove(SdFile* parent_dir,
                        const char* file_path_component,
                        bool is_last_component,
                        __attribute__((unused)) void* object)
{
    if (is_last_component) {
        return sd_file_remove_path(parent_dir, file_path_component);
    }
    return true;
}


bool sd_callback_rmdir(SdFile* parent_dir,
                       const char* file_path_component,
                       bool is_last_component,
                       __attribute__((unused)) void* object)
{
    if (is_last_component) {
        SdFile f;
        sd_file_init(&f);
        if (!sd_file_open(&f, parent_dir, file_path_component, O_READ)) {
            return false;
        }
        return sd_file_rm_dir(&f);
    }
    return true;
}


bool sd_walk_path(const char* filepath, SdFile* parent_dir,
                  sd_callback_f callback, void* object)
{
    SdFile subfile1;
    SdFile subfile2;
    sd_file_init(&subfile1);
    sd_file_init(&subfile2);

    char buffer[PATH_COMPONENT_BUFFER_LEN];

    unsigned int offset = 0;

    SdFile* p_parent;
    SdFile* p_child;

    SdFile* p_tmp_sdfile;

    p_child = &subfile1;
    p_parent = parent_dir;

    while (true) {
        bool more_components = get_next_path_component(filepath, &offset, buffer);
        bool should_continue = callback(p_parent, buffer, !more_components, object);

        if (!should_continue) {
            // TODO: Don't repeat this code? If it's one we've created then we
            // don't need the parent handle anymore.
            if (p_parent != parent_dir) {
                sd_file_close(p_parent);
            }
            return false;
        }

        if (!more_components) {
            break;
        }

        bool exists = sd_file_open(p_child, p_parent, buffer, O_RDONLY);

        // If it's one we've created, we don't need the parent handle anymore
        if (p_parent != parent_dir) {
            sd_file_close(p_parent);
        }

        // Handle case when it doesn't exist and we can't continue...
        if (exists) {
            // We alternate between two file handles as we go down the path.
            if (p_parent == parent_dir) {
                p_parent = &subfile2;
            }

            p_tmp_sdfile = p_parent;
            p_parent = p_child;
            p_child = p_tmp_sdfile;
        } else {
            return false;
        }
    }

    if (p_parent != parent_dir) {
        sd_file_close(p_parent); // TODO: Return/ handle different?
    }

    return true;
}
