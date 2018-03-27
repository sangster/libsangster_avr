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


void sd_file_ls(SdFile* file, uint8_t flags, uint8_t indent)
{
    SdDir* p;

    sd_file_rewind(file);
    while ((p = sd_file_read_dir_cache(file))) {
        // done if past last used entry
        if (p->name[0] == DIR_NAME_FREE) {
            break;
        }

        // skip deleted entry and entries for . and  ..
        if (p->name[0] == DIR_NAME_DELETED || p->name[0] == '.') {
            continue;
        }

        // only list subdirectories and files
        if (!DIR_IS_FILE_OR_SUBDIR(p)) {
            continue;
        }

        // print any indent spaces
        for (int8_t i = 0; i < indent; i++) {
            usart_send(' ');
        }

        // print file name with possible blank fill
        sd_file_print_dir_name(*p, flags & (LS_DATE | LS_SIZE) ? 14 : 0);

        // print modify date/time if requested
        if (flags & LS_DATE) {
            sd_file_print_fat_date(p->last_write_date);
            usart_send(' ');
            sd_file_print_fat_time(p->last_write_time);
        }
        // print size if requested
        if (!DIR_IS_SUBDIR(p) && (flags & LS_SIZE)) {
            usart_send(' ');
            usart_32(p->file_size);
        }
        usart_crlf();

        // list subdirectory content if requested
        if ((flags & LS_R) && DIR_IS_SUBDIR(p)) {
            uint16_t index = file->cur_position / 32 - 1;
            SdFile s;
            sd_file_init(&s);
            if (sd_file_open_by_index(&s, file, index, O_READ)) {
                sd_file_ls(&s, flags, indent + 2);
            }
            sd_file_seek_set(file, 32 * (index + 1));
        }
    }
}


// Read next directory entry into the cache
// Assumes file is correctly positioned
SdDir* sd_file_read_dir_cache(SdFile* file)
{
    // error if not directory
    if (!sd_file_is_dir(file)) {
        return NULL;
    }

    // index of entry in cache
    uint8_t i = (file->cur_position >> 5) & 0xF;

    // use read to locate and cache block
    if (sd_file_read_next(file) < 0) {
        return NULL;
    }

    // advance to next entry
    file->cur_position += 31;

    // return pointer to entry
    return (cache_buffer.dir + i);
}


void sd_file_print_dir_name(const SdDir dir, uint8_t width)
{
    uint8_t w = 0;
    for (uint8_t i = 0; i < 11; i++) {
        if (dir.name[i] == ' ') {
            continue;
        }
        if (i == 8) {
            usart_send('.');
            w++;
        }
        usart_send(dir.name[i]);
        w++;
    }
    if (DIR_IS_SUBDIR(&dir)) {
        usart_send('/');
        w++;
    }
    while (w < width) {
        usart_send(' ');
        w++;
    }
}


// open a cached directory entry. Assumes file->vol is initialized
uint8_t sd_file_open_cached_entry(SdFile* file, uint8_t dir_index,
                                  uint8_t oflag)
{
    // location of entry in cache
    SdDir* p = cache_buffer.dir + dir_index;

    // write or truncate is an error for a directory or read-only file
    if (p->attributes & (DIR_ATT_READ_ONLY | DIR_ATT_DIRECTORY)) {
        if (oflag & (O_WRITE | O_TRUNC)) {
            return false;
        }
    }
    // remember location of directory entry on SD
    file->dir_index = dir_index;
    file->dir_block = cache_block_number;

    // copy first cluster number for directory fields
    file->first_cluster = (uint32_t) p->first_cluster_high << 16;
    file->first_cluster |= p->first_cluster_low;

    // make sure it is a normal file or subdirectory
    if (DIR_IS_FILE(p)) {
        file->file_size = p->file_size;
        file->type = FAT_FILE_TYPE_NORMAL;
    } else if (DIR_IS_SUBDIR(p)) {
        if (!sd_volume_chain_size(file->vol, file->first_cluster, &file->file_size)) {
            return false;
        }
        file->type = FAT_FILE_TYPE_SUBDIR;
    } else {
        return false;
    }
    // save open flags for read/write
    file->flags = oflag & (O_ACCMODE | O_SYNC | O_APPEND);

    // set to start of file
    file->cur_cluster = 0;
    file->cur_position = 0;
    file->write_error = 0;

    // truncate file to zero length if requested
    if (oflag & O_TRUNC) {
        return sd_file_truncate(file, 0);
    }
    return true;
}


uint8_t sd_file_add_dir_cluster(SdFile* file)
{
    if (!sd_file_add_cluster(file)) {
        return false;
    }

    // zero data in cluster insure first cluster is in cache
    uint32_t block = sd_volume_cluster_start_block(file->vol, file->cur_cluster);

    for (uint8_t i = file->vol->blocks_per_cluster; i != 0; i--) {
        if (!sd_volume_cache_zero_block(block + i - 1)) {
            return false;
        }
    }
    // Increase directory file size by cluster size
    file->file_size += 512UL << file->vol->cluster_size_shift;
    return true;
}


SdDir* sd_file_cache_dir_entry(SdFile* file, uint8_t action)
{
    if (!sd_volume_cache_raw_block(file->dir_block, action)) {
        return NULL;
    }
    return cache_buffer.dir + file->dir_index;
}


uint8_t sd_file_dir_entry(SdFile* file, SdDir* dir)
{
    // make sure fields on SD are correct
    if (!sd_file_sync(file)) {
        return false;
    }

    // read entry
    SdDir* p = sd_file_cache_dir_entry(file, CACHE_FOR_READ);
    if (!p) {
        return false;
    }

    // copy to caller's struct
    memcpy(dir, p, sizeof(SdDir));
    return true;
}


void sd_file_dir_name(const SdDir* dir, char* name)
{
    uint8_t j = 0;
    for (uint8_t i = 0; i < 11; i++) {
        if (dir->name[i] == ' ') {
            continue;
        }
        if (i == 8) {
            name[j++] = '.';
        }
        name[j++] = dir->name[i];
    }
    name[j] = 0;
}


uint8_t sd_file_make_dir(SdFile* file, SdFile* dir, const char* dir_name)
{
    SdDir d;

    // create a normal file
    if (!sd_file_open(file, dir, dir_name, O_CREAT | O_EXCL | O_RDWR)) {
        return false;
    }

    // convert SdFile to directory
    file->flags = O_READ;
    file->type = FAT_FILE_TYPE_SUBDIR;

    // allocate and zero first cluster
    if (!sd_file_add_dir_cluster(file)) {
        return false;
    }

    // force entry to SD
    if (!sd_file_sync(file)) {
        return false;
    }

    // cache entry - should already be in cache due to sync() call
    SdDir* p = sd_file_cache_dir_entry(file, CACHE_FOR_WRITE);
    if (!p) {
        return false;
    }

    // change directory entry  attribute
    p->attributes = DIR_ATT_DIRECTORY;

    // make entry for '.'
    memcpy(&d, p, sizeof(d));
    for (uint8_t i = 1; i < 11; i++) {
        d.name[i] = ' ';
    }
    d.name[0] = '.';

    // cache block for '.'  and '..'
    uint32_t block = sd_volume_cluster_start_block(file->vol, file->first_cluster);
    if (!sd_volume_cache_raw_block(block, CACHE_FOR_WRITE)) {
        return false;
    }

    // copy '.' to block
    memcpy(&cache_buffer.dir[0], &d, sizeof(d));

    // make entry for '..'
    d.name[1] = '.';
    if (sd_file_is_root(dir)) {
        d.first_cluster_low = 0;
        d.first_cluster_high = 0;
    } else {
        d.first_cluster_low = dir->first_cluster & 0xFFFF;
        d.first_cluster_high = dir->first_cluster >> 16;
    }

    memcpy(&cache_buffer.dir[1], &d, sizeof(d)); // copy '..' to block
    file->cur_position = 2 * sizeof(d);          // set position after '..'
    return sd_volume_cache_flush();              // write first block
}


int8_t sd_file_read_dir(SdFile* file, SdDir* dir)
{
    int8_t n;
    // if not a directory file or miss-positioned return an error
    if (!sd_file_is_dir(file) || (0x1F & file->cur_position)) {
        return -1;
    }

    while ((n = sd_file_read(file, (uint8_t*) dir, sizeof(SdDir))) == sizeof(SdDir)) {
        // last entry if DIR_NAME_FREE
        if (dir->name[0] == DIR_NAME_FREE) {
            break;
        }
        // skip empty entries and entry for . and ..
        if (dir->name[0] == DIR_NAME_DELETED || dir->name[0] == '.') {
            continue;
        }
        // return if normal file or subdirectory
        if (DIR_IS_FILE_OR_SUBDIR(dir)) {
            return n;
        }
    }

    // error, end of file, or past last entry
    return n < 0 ? -1 : 0;
}
