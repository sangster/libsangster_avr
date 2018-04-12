#ifndef SANGSTER_SD_H
#define SANGSTER_SD_H
/*
 * Arduino SdFat Library
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
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * the Arduino SdFat Library. If not, see <http://www.gnu.org/licenses/>.
 */
/**
 * @file
 *
 * SD - a slightly more friendly wrapper for sdfatlib
 *
 * This library aims to expose a subset of SD card functionality
 * in the form of a higher level "wrapper" object.
 *
 * License: GNU General Public License V3
 *          (Because sdfatlib is licensed with this.)
 *
 * (C) Copyright 2010 SparkFun Electronics
 */

#include <stdbool.h>
#include "sangster/api.h"
#include "sangster/pinout.h"
#include "sangster/sd/sd_card.h"
#include "sangster/sd/sd_file.h"
#include "sangster/sd/sd_volume.h"


/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define FILE_READ O_READ
#define FILE_WRITE (O_READ | O_WRITE | O_CREAT)

// Used by `getNextPathComponent`
#define MAX_COMPONENT_LEN 12 // What is max length?
#define PATH_COMPONENT_BUFFER_LEN (MAX_COMPONENT_LEN + 1)
#define RETURN_ERR_FILE do { SdFile err; sd_file_init(&err); return err; }while(0)


/*******************************************************************************
 * Types
 ******************************************************************************/
typedef struct sd_class SdClass;
struct sd_class
{
    SdCard card;
    SdVolume volume;
    SdFile root;

    // This is used to determine the mode used to open a file it's here because
    // it's the easiest place to pass the information through the directory
    // walking function. But it's probably not the best place for it. It
    // shouldn't be set directly--it is set via the parameters to `open`.
    int file_open_mode;
};

typedef bool (*SdWalkPathFunc)(SdFile* parent_dir,
                               const char* file_path_component,
                               bool is_last_component, void* object);


/*******************************************************************************
 * Function Declarations
 ******************************************************************************/
/*
 * Performs the initialisation required by the sdfatlib library.
 *
 * Return true if initialization succeeds, false otherwise.
 */
SA_FUNC bool sd_begin(SdClass*, SdFileDateTime);

/**
 * Call this when a card is removed. It will allow you to inster and initialise
 * a new card.
 */
SA_FUNC void sd_end(SdClass*);

/**
 * This little helper is used to traverse paths.
 *
 * @param [in]  sd
 * @param [in]  filepath
 * @param [out] index
 */
SA_FUNC SdFile sd_get_parent_dir(SdClass*, const char*, int*)

/*
 * Open the supplied file path for reading or writing.
 *
 * The file content can be accessed via the `file` property of the `SDClass`
 * object--this property is currently a standard `SdFile` object from
 * `sdfatlib`.
 *
 * Defaults to read only.
 *
 * If `write` is true, default action (when `append` is true) is to append data
 * to the end of the file.
 *
 * If `append` is false then the file will be truncated first.
 *
 * If the file does not exist and it is opened for writing the file will be
 * created.
 *
 * An attempt to open a file for reading that does not exist is an error.
 */
SA_FUNC SdFile sd_open(SdClass*, const char*, uint8_t);

/**
 * @defgroup sd_callbacks Callback functions used by sd_walk_path
 *
 *  The callbacks used to implement various functionality follow.
 *
 *  Each callback is supplied with a parent directory handle, character string
 *  with the name of the current file path component, a flag indicating if this
 *  component is the last in the path and a pointer to an arbitrary object used
 *  for context.
 */
/// @{

/*
 * Callback used to determine if a file/directory exists in parent directory.
 *
 * Returns true if file path exists.
 */
SA_FUNC bool sd_callback_path_exists(SdFile*, const char*,
                                     __attribute__((unused)) bool,
                                     __attribute__((unused)) void*);

/*
 * Callback used to create a directory in the parent directory if it does not
 * already exist.
 *
 * Returns true if a directory was created or it already existed.
 */
SA_FUNC bool sd_callback_make_dir_path(SdFile*, const char*, bool, void*);

SA_FUNC bool sd_callback_remove(SdFile*, const char*, bool,
                                __attribute__((unused)) void*);

SA_FUNC bool sd_callback_rmdir(SdFile*, const char*, bool,
                              __attribute__((unused)) void*);
/// @}

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
SA_FUNC bool get_next_path_component(const char*, unsigned int*, char*);

/*
 * When given a file path (and parent directory--normally root), this function
 * traverses the directories in the path and at each level calls the supplied
 * callback function while also providing the supplied object for context if
 * required.
 *
 * e.g. given the path '/foo/bar/baz' the callback would be called at the
 * equivalent of '/foo', '/foo/bar' and '/foo/bar/baz'.
 *
 * The implementation swaps between two different directory/file handles as it
 * traverses the directories and does not use recursion in an attempt to use
 * memory efficiently.
 *
 * If a callback wishes to stop the directory traversal it should return
 * false--in this case the function will stop the traversal, tidy up and return
 * false.
 *
 * If a directory path doesn't exist at some point this function will also
 * return false and not subsequently call the callback.
 *
 * If a directory path specified is complete, valid and the callback did not
 * indicate the traversal should be interrupted then this function will return
 * true.
 */
SA_FUNC bool sd_walk_path(const char*, SdFile*, SdWalkPathFunc, void*);

/// @return `true` if the supplied file path exists in the given dir
SA_INLINE bool sd_exists_in_dir(SdFile*, const char*);

/// Returns true if the supplied file path exists
SA_INLINE bool sd_exists(SdClass*, const char*);

/*
 * Makes a single directory or a heirarchy of directories. A rough equivalent
 * to `mkdir -p`.
 */
SA_INLINE bool sd_mkdir(SdClass*, const char*);

SA_INLINE bool sd_rmdir(SdClass*, const char*);

SA_INLINE bool sd_remove(SdClass*, const char*);


/*******************************************************************************
 * Function Definitions
 ******************************************************************************/
SA_FUNC bool sd_begin(SdClass* sd, SdFileDateTime date_time_callback)
{
    sd_date_time_callback = date_time_callback;
    cache_block_number = 0xFFFFFFFF;
    cache_dirty = 0;
    cache_mirror_block = 0;

    return sd_card_init(&(sd->card), SPI_HALF_SPEED)
        && sd_volume_init_try_both(&(sd->volume), &(sd->card))
        && sd_file_open_root(&(sd->root), &(sd->volume));
}


SA_FUNC void sd_end(SdClass* sd)
{
    sd_file_close(&sd->root);
}


SA_FUNC SdFile sd_get_parent_dir(SdClass* sd, const char* filepath, int* index)
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


SA_FUNC SdFile sd_open(SdClass* sd, const char* filepath, uint8_t mode)
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

SA_FUNC bool sd_callback_path_exists(SdFile* parent_dir,
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


SA_FUNC bool sd_callback_make_dir_path(SdFile* parent_dir,
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


SA_FUNC bool sd_callback_remove(SdFile* parent_dir,
                               const char* file_path_component,
                               bool is_last_component,
                               __attribute__((unused)) void* object)
{
    if (is_last_component) {
        return sd_file_remove_path(parent_dir, file_path_component);
    }
    return true;
}


SA_FUNC bool sd_callback_rmdir(SdFile* parent_dir,
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


SA_FUNC bool get_next_path_component(const char* path, unsigned int* p_offset,
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


SA_FUNC bool sd_walk_path(const char* filepath, SdFile* parent_dir,
                          SdWalkPathFunc callback, void* object)
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


SA_INLINE bool sd_exists_in_dir(SdFile* dir, const char* filepath)
{
    return sd_walk_path(filepath, dir, sd_callback_path_exists, NULL);
}


SA_INLINE bool sd_exists(SdClass* sd, const char* filepath)
{
    return sd_exists_in_dir(&sd->root, filepath);
}


SA_INLINE bool sd_mkdir(SdClass* sd, const char* filepath)
{
    return sd_walk_path(filepath, &sd->root, sd_callback_make_dir_path, NULL);
}


SA_INLINE bool sd_rmdir(SdClass* sd, const char* filepath)
{
    return sd_walk_path(filepath, &sd->root, sd_callback_rmdir, NULL);
}


SA_INLINE bool sd_remove(SdClass* sd, const char* filepath)
{
    return sd_walk_path(filepath, &sd->root, sd_callback_remove, NULL);
}

#endif//SANGSTER_SD_H
