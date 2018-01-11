#ifndef SANGSTER_SD_H
#define SANGSTER_SD_H
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
#include "sangster/pinout.h"
#include "sangster/sd/sd_card.h"
#include "sangster/sd/sd_file.h"
#include "sangster/sd/sd_volume.h"

#define FILE_READ O_READ
#define FILE_WRITE (O_READ | O_WRITE | O_CREAT)

// Used by `getNextPathComponent`
#define MAX_COMPONENT_LEN 12 // What is max length?
#define PATH_COMPONENT_BUFFER_LEN (MAX_COMPONENT_LEN + 1)


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

typedef bool (*sd_callback_f)(SdFile* parent_dir,
                              const char* file_path_component,
                              bool is_last_component, void* object);


/*
 * Performs the initialisation required by the sdfatlib library.
 *
 * Return true if initialization succeeds, false otherwise.
 */
bool sd_begin(SdClass* sd, SdFileDateTime);


//call this when a card is removed. It will allow you to inster and initialise
//a new card.
void sd_end(SdClass* sd);


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
SdFile sd_open(SdClass* sd, const char* filepath, uint8_t mode);


// this little helper is used to traverse paths
SdFile sd_get_parent_dir(SdClass* sd, const char* filepath, int* index);


/*******************************************************************************
 *  The callbacks used to implement various functionality follow.
 *
 *  Each callback is supplied with a parent directory handle, character string
 *  with the name of the current file path component, a flag indicating if this
 *  component is the last in the path and a pointer to an arbitrary object used
 *  for context.
 ******************************************************************************/

/*
 * Callback used to determine if a file/directory exists in parent directory.
 *
 * Returns true if file path exists.
 */
bool sd_callback_path_exists(SdFile* parent_dir, const char* file_path_component,
                             bool is_last_component, void* object);


/*
 * Callback used to create a directory in the parent directory if it does not
 * already exist.
 *
 * Returns true if a directory was created or it already existed.
 */
bool sd_callback_make_dir_path(SdFile* parent_dir, const char* file_path_component,
                               bool is_last_component, void* object);


bool sd_callback_remove(SdFile* parent_dir, const char* file_path_component,
                        bool is_last_component, void* object);


bool sd_callback_rmdir(SdFile* parent_dir, const char* file_path_component,
                       bool is_last_component, void* object);


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
bool sd_walk_path(const char* filepath, SdFile* parent_dir,
                  sd_callback_f callback, void* object);


/* Returns true if the supplied file path exists in the given dir. */
static inline bool sd_exists_in_dir(SdFile* dir, const char* filepath)
{
    return sd_walk_path(filepath, dir, sd_callback_path_exists, NULL);
}


/* Returns true if the supplied file path exists. */
static inline bool sd_exists(SdClass* sd, const char* filepath)
{
    return sd_exists_in_dir(&sd->root, filepath);
}


/*
 * Makes a single directory or a heirarchy of directories. A rough equivalent
 * to `mkdir -p`.
 */
static inline bool sd_mkdir(SdClass* sd, const char* filepath)
{
    return sd_walk_path(filepath, &sd->root, sd_callback_make_dir_path, NULL);
}


static inline bool sd_rmdir(SdClass* sd, const char* filepath)
{
    return sd_walk_path(filepath, &sd->root, sd_callback_rmdir, NULL);
}


static inline bool sd_remove(SdClass* sd, const char* filepath)
{
    return sd_walk_path(filepath, &sd->root, sd_callback_remove, NULL);
}

#endif//SANGSTER_SD_H
