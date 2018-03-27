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
#include "sd_file.h"


uint8_t sd_file_open_root(SdFile* file, SdVolume* vol)
{
    if (sd_file_is_open(file)) { // error if file is already open
        return false;
    }

    if (vol->fat_type == 16) {
        file->type = FAT_FILE_TYPE_ROOT16;
        file->first_cluster = 0;
        file->file_size = 32 * vol->root_dir_entry_count;
    } else if (vol->fat_type == 32) {
        file->type = FAT_FILE_TYPE_ROOT32;
        file->first_cluster = vol->root_dir_start;

        if (!sd_volume_chain_size(vol, file->first_cluster, &(file->file_size))) {
            return false;
        }
    } else {
        return false; // volume is not initialized or FAT12
    }

    file->vol = vol;
    file->flags = O_READ; // read only

    // set to start of file
    file->cur_cluster = 0;
    file->cur_position = 0;

    // root has no directory entry
    file->dir_block = 0;
    file->dir_index = 0;

    return true;
}


uint8_t sd_file_open(SdFile* file, SdFile* dir, const char* file_name,
                     uint8_t oflag)
{
    uint8_t dname[11];
    SdDir* p;

    // error if already open
    if (sd_file_is_open(file)) {
        return false;
    }

    if (!sd_file_make83_name(file_name, dname)) {
        return false;
    }
    file->vol = dir->vol;
    sd_file_rewind(dir);

    // bool for empty entry found
    uint8_t empty_found = false;

    // search for file
    while (dir->cur_position < dir->file_size) {
        uint8_t index = 0xF & (dir->cur_position >> 5);
        p = sd_file_read_dir_cache(dir);
        if (p == NULL) {
            return false;
        }

        if (p->name[0] == DIR_NAME_FREE || p->name[0] == DIR_NAME_DELETED) {
            // remember first empty slot
            if (!empty_found) {
                empty_found = true;
                file->dir_index = index;
                file->dir_block = cache_block_number;
            }
            // done if no entries follow
            if (p->name[0] == DIR_NAME_FREE) {
                break;
            }
        } else if (!memcmp(dname, p->name, 11)) {
            // don't open existing file if O_CREAT and O_EXCL
            if ((oflag & (O_CREAT | O_EXCL)) == (O_CREAT | O_EXCL)) {
                return false;
            }

            // open found file
            return sd_file_open_cached_entry(file, 0xF & index, oflag);
        }
    }
    // only create file if O_CREAT and O_WRITE
    if ((oflag & (O_CREAT | O_WRITE)) != (O_CREAT | O_WRITE)) {
        return false;
    }

    // cache found slot or add cluster if end of file
    if (empty_found) {
        p = sd_file_cache_dir_entry(file, CACHE_FOR_WRITE);
        if (!p) {
            return false;
        }
    } else {
        if (dir->type == FAT_FILE_TYPE_ROOT16) {
            return false;
        }

        // add and zero cluster for dir - first cluster is in cache for write
        if (!sd_file_add_dir_cluster(dir)) {
            return false;
        }

        // use first entry in cluster
        file->dir_index = 0;
        p = cache_buffer.dir;
    }
    // initialize as empty file
    memset(p, 0, sizeof(SdDir));
    memcpy(p->name, dname, 11);

    if (file->date_time == NULL) {
        file->date_time =
            dir->date_time != NULL ? dir->date_time : sd_date_time_callback;
    }

    // set timestamps
    if (file->date_time != NULL) {
        // call user function
        (*file->date_time)(&p->creation_date, &p->creation_time);
    } else {
        // use default date/time
        p->creation_date = FAT_DEFAULT_DATE;
        p->creation_time = FAT_DEFAULT_TIME;
    }
    p->last_access_date = p->creation_date;
    p->last_write_date  = p->creation_date;
    p->last_write_time  = p->creation_time;

    // force write of entry to SD
    if (!sd_volume_cache_flush()) {
        return false;
    }

    // open entry in cache
    return sd_file_open_cached_entry(file, file->dir_index, oflag);
}


uint8_t sd_file_open_by_index(SdFile* file, SdFile* dir, uint16_t index,
                              uint8_t oflag)
{
    // error if already open
    if (sd_file_is_open(file)) {
        return false;
    }

    // don't open existing file if O_CREAT and O_EXCL - user call error
    if ((oflag & (O_CREAT | O_EXCL)) == (O_CREAT | O_EXCL)) {
        return false;
    }

    file->vol = dir->vol;

    // seek to location of entry
    if (!sd_file_seek_set(dir, 32 * index)) {
        return false;
    }

    // read entry into cache
    SdDir* p = sd_file_read_dir_cache(dir);
    if (p == NULL) {
        return false;
    }

    // error if empty slot or '.' or '..'
    const uint8_t ch = p->name[0];
    if (ch == DIR_NAME_FREE || ch == DIR_NAME_DELETED || ch == '.') {
        return false;
    }
    // open cached entry
    return sd_file_open_cached_entry(file, index & 0xF, oflag);
}


uint8_t sd_file_close(SdFile* file)
{
    if(!sd_file_sync(file)) {
        return false;
    }
    file->type = FAT_FILE_TYPE_CLOSED;
    return true;
}
