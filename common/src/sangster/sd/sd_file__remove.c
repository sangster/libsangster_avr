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


uint8_t sd_file_remove(SdFile* file)
{
    // free any clusters - will fail if read-only or directory
    if (!sd_file_truncate(file, 0)) {
        return false;
    }

    // cache directory entry
    SdDir* d = sd_file_cache_dir_entry(file, CACHE_FOR_WRITE);
    if (!d) {
        return false;
    }

    d->name[0] = DIR_NAME_DELETED;     // mark entry deleted
    file->type = FAT_FILE_TYPE_CLOSED; // set this SdFile closed
    return sd_volume_cache_flush();    // write entry to SD
}


uint8_t sd_file_remove_path(SdFile* dir, const char* file_name)
{
  SdFile file;
  sd_file_init(&file);

  if (!sd_file_open(&file, dir, file_name, O_WRITE)) {
      return false;
  }
  return sd_file_remove(&file);
}


uint8_t sd_file_rm_dir(SdFile* file)
{
    // must be open subdirectory
    if (!sd_file_is_subdir(file)) {
        return false;
    }

    sd_file_rewind(file);

    // make sure directory is empty
    while (file->cur_position < file->file_size) {
        SdDir* p = sd_file_read_dir_cache(file);
        if (p == NULL) {
            return false;
        }
        // done if past last used entry
        if (p->name[0] == DIR_NAME_FREE) {
            break;
        }
        // skip empty slot or '.' or '..'
        if (p->name[0] == DIR_NAME_DELETED || p->name[0] == '.') {
            continue;
        }
        // error not empty
        if (DIR_IS_FILE_OR_SUBDIR(p)) {
            return false;
        }
    }
    // convert empty directory to normal file for remove
    file->type = FAT_FILE_TYPE_NORMAL;
    file->flags |= O_WRITE;
    return sd_file_remove(file);
}


uint8_t sd_file_rm_rf(SdFile* file)
{
    sd_file_rewind(file);
    while (file->cur_position < file->file_size) {
        SdFile f;
        sd_file_init(&f);

        // remember position
        uint16_t index = file->cur_position / 32;

        SdDir* p = sd_file_read_dir_cache(file);
        if (!p) {
            return false;
        }

        // done if past last entry
        if (p->name[0] == DIR_NAME_FREE) {
            break;
        }

        // skip empty slot or '.' or '..'
        if (p->name[0] == DIR_NAME_DELETED || p->name[0] == '.') {
            continue;
        }

        // skip if part of long file name or volume label in root
        if (!DIR_IS_FILE_OR_SUBDIR(p)) {
            continue;
        }

        if (!sd_file_open_by_index(&f, file, index, O_READ)) {
            return false;
        }
        if (sd_file_is_subdir(&f)) {
            // recursively delete
            if (!sd_file_rm_rf(&f)) {
                return false;
            }
        } else {
            // ignore read-only
            f.flags |= O_WRITE;
            if (!sd_file_remove(&f)) {
                return false;
            }
        }
        // position to next entry if required
        if (file->cur_position != (32 * (index + 1))) {
            if (!sd_file_seek_set(file, 32 * (index + 1))) {
                return false;
            }
        }
    }
    // don't try to delete root
    if (sd_file_is_root(file)) {
        return true;
    }
    return sd_file_rm_dir(file);
}
