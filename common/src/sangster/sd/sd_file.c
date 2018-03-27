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


SdFileDateTime sd_date_time_callback = NULL;


int16_t sd_file_read(SdFile* file, uint8_t* dst, uint16_t nbyte)
{
    // error if not open or write only
    if (!sd_file_is_open(file) || !(file->flags & O_READ)) {
        return -1;
    }
    if (file->cur_position == file->file_size) {
        return -1; // end of file
    }

    // max bytes left in file
    if (nbyte > (file->file_size - file->cur_position)) {
        nbyte = file->file_size - file->cur_position;
    }

    // amount left to read
    uint16_t to_read = nbyte;
    while (to_read > 0) {
        uint32_t block;  // raw device block number
        uint16_t offset = file->cur_position & 0x1FF;  // offset in block

        if (file->type == FAT_FILE_TYPE_ROOT16) {
            block = file->vol->root_dir_start + (file->cur_position >> 9);
        } else {
            uint8_t block_of_cluster =
                sd_volume_block_of_cluster(file->vol, file->cur_position);
            if (offset == 0 && block_of_cluster == 0) {
                // start of new cluster
                if (file->cur_position == 0) {
                    // use first cluster in file
                    file->cur_cluster = file->first_cluster;
                } else {
                    // get next cluster from FAT
                    if (!sd_volume_fat_get(file->vol, file->cur_cluster,
                                           &file->cur_cluster)) {
                        return -1;
                    }
                }
            }
            block = sd_volume_cluster_start_block(file->vol, file->cur_cluster)
                + block_of_cluster;
        }
        uint16_t n = to_read;

        // amount to be read from current block
        if (n > 512 - offset) {
            n = 512 - offset;
        }

        // no buffering needed if n == 512 or user requests no buffering
        if ((sd_file_unbuffered_read(file) || n == 512) && block != cache_block_number) {
            if (!sd_card_read_data(sd_card, block, offset, n, dst)) {
                return -1;
            }
            dst += n;
        } else {
            // read block to cache and copy data to caller
            if (!sd_volume_cache_raw_block(block, CACHE_FOR_READ)) {
                return -1;
            }
            uint8_t* src = cache_buffer.data + offset;
            uint8_t* end = src + n;
            while (src != end) {
                *dst++ = *src++;
            }
        }
        file->cur_position += n;
        to_read -= n;
    }
    return nbyte;
}


uint8_t sd_file_make83_name(const char* str, uint8_t* name)
{
    uint8_t c;
    uint8_t n = 7;  // max index for part before dot
    uint8_t i = 0;
    // blank fill name and extension
    while (i < 11) {
        name[i++] = ' ';
    }
    i = 0;
    while ((c = *str++) != '\0') {
        if (c == '.') {
            if (n == 10) {
                return false;  // only one dot allowed
            }
            n = 10;  // max index for full 8.3 name
            i = 8;   // place for extension
        } else {
            // illegal FAT characters
            uint8_t b;
            PGM_P p = PSTR("|<>^+=?/[];,*\"\\");
            while ((b = pgm_read_byte(p++))) {
                if (b == c) {
                    return false;
                }
            }
            // check size and only allow ASCII printable characters
            if (i > n || c < 0x21 || c > 0x7E) {
                return false;
            }
            // only upper case allowed in 8.3 names - convert lower to upper
            name[i++] = c < 'a' || c > 'z' ?  c : c + ('A' - 'a');
        }
    }
    // must have a file name, extension is optional
    return name[0] != ' ';
}


uint8_t sd_file_truncate(SdFile* file, uint32_t length)
{
    // error if not a normal file or read-only
    if (!sd_file_is_file(file) || !(file->flags & O_WRITE)) {
        return false;
    }

    // error if length is greater than current size
    if (length > file->file_size) {
        return false;
    }

    // fileSize and length are zero - nothing to do
    if (file->file_size == 0) {
        return true;
    }

    // remember position for seek after truncation
    uint32_t new_pos = file->cur_position > length ? length : file->cur_position;

    // position to last cluster in truncated file
    if (!sd_file_seek_set(file, length)) {
        return false;
    }

    if (length == 0) {
        // free all clusters
        if (!sd_volume_free_chain(file->vol, file->first_cluster)) {
            return false;
        }
        file->first_cluster = 0;
    } else {
        uint32_t to_free;
        if (!sd_volume_fat_get(file->vol, file->cur_cluster, &to_free)) {
            return false;
        }

        if (!sd_volume_is_eoc(file->vol, to_free)) {
            // free extra clusters
            if (!sd_volume_free_chain(file->vol, to_free)) {
                return false;
            }

            // current cluster is end of chain
            if (!sd_volume_fat_put_eoc(file->vol, file->cur_cluster)) {
                return false;
            }
        }
    }
    file->file_size = length;

    // need to update directory entry
    file->flags |= F_FILE_DIR_DIRTY;

    if (!sd_file_sync(file)) {
        return false;
    }

    // set file to correct position
    return sd_file_seek_set(file, new_pos);
}


/**
 * Sets a file's position.
 *
 * @param[in] pos The new position in bytes from the beginning of the file.
 *
 * @return The value one, true, is returned for success and the value zero,
 *   false, is returned for failure.
 */
uint8_t sd_file_seek_set(SdFile* file, uint32_t pos)
{
    // error if file not open or seek past end of file
    if (!sd_file_is_open(file) || pos > file->file_size) {
        return false;
    }

    if (file->type == FAT_FILE_TYPE_ROOT16) {
        file->cur_position = pos;
        return true;
    }
    if (pos == 0) {
        // set position to start of file
        file->cur_cluster = 0;
        file->cur_position = 0;
        return true;
    }
    // calculate cluster index for cur and new position
    uint32_t n_cur = (file->cur_position - 1) >> (file->vol->cluster_size_shift + 9);
    uint32_t n_new = (pos - 1) >> (file->vol->cluster_size_shift + 9);

    if (n_new < n_cur || file->cur_position == 0) {
        // must follow chain from first cluster
        file->cur_cluster = file->first_cluster;
    } else {
        // advance from curPosition
        n_new -= n_cur;
    }
    while (n_new--) {
        if (!sd_volume_fat_get(file->vol, file->cur_cluster, &file->cur_cluster)) {
            return false;
        }
    }
    file->cur_position = pos;
    return true;
}


uint8_t sd_file_sync(SdFile* file)
{
    // only allow open files and directories
    if (!sd_file_is_open(file)) {
        return false;
    }

    if (file->flags & F_FILE_DIR_DIRTY) {
        SdDir* d = sd_file_cache_dir_entry(file, CACHE_FOR_WRITE);
        if (!d) {
            return false;
        }

        // do not set filesize for dir files
        if (!sd_file_is_dir(file)) {
            d->file_size = file->file_size;
        }

        // update first cluster fields
        d->first_cluster_low = file->first_cluster & 0xFFFF;
        d->first_cluster_high = file->first_cluster >> 16;

        // set modify time if user supplied a callback date/time function
        if (file->date_time) {
            (*file->date_time)(&d->last_write_date, &d->last_write_time);
            d->last_access_date = d->last_write_date;
        }
        // clear directory dirty
        file->flags &= ~F_FILE_DIR_DIRTY;
    }
    return sd_volume_cache_flush();
}


uint8_t sd_file_add_cluster(SdFile* file)
{
    if (!sd_volume_alloc_contiguous(file->vol, 1, &file->cur_cluster)) {
        return false;
    }

    // if first cluster of file link to directory entry
    if (file->first_cluster == 0) {
        file->first_cluster = file->cur_cluster;
        file->flags |= F_FILE_DIR_DIRTY;
    }
    return true;
}


int16_t sd_file_read_next(SdFile* file)
{
    uint8_t b;
    return sd_file_read(file, &b, 1) == 1 ? b : -1;
}
