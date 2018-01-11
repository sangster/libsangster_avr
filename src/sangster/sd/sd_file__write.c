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


size_t sd_file_write(SdFile* file, const uint8_t* src, uint16_t nbyte)
{
    // number of bytes left to write - must be before goto statements
    uint16_t n_to_write = nbyte;

    // error if not a normal file or is read-only
    if (!sd_file_is_file(file) || !(file->flags & O_WRITE)) {
        goto write_error_return;
    }

    // seek to end of file if append flag
    if ((file->flags & O_APPEND) && file->cur_position != file->file_size) {
        if (!sd_file_seek_set(file, file->file_size)) {
            goto write_error_return;
        }
    }

    while (n_to_write > 0) {
        uint8_t block_of_cluster =
            sd_volume_block_of_cluster(file->vol, file->cur_position);
        uint16_t block_offset = file->cur_position & 0x1FF;
        if (block_of_cluster == 0 && block_offset == 0) {
            // start of new cluster
            if (file->cur_cluster == 0) {
                if (file->first_cluster == 0) {
                    // allocate first cluster of file
                    if (!sd_file_add_cluster(file)) {
                        goto write_error_return;
                    }
                } else {
                    file->cur_cluster = file->first_cluster;
                }
            } else {
                uint32_t next;
                if (!sd_volume_fat_get(file->vol, file->cur_cluster, &next)) {
                    return false;
                }
                if (sd_volume_is_eoc(file->vol, next)) {
                    // add cluster if at end of chain
                    if (!sd_file_add_cluster(file)) {
                        goto write_error_return;
                    }
                } else {
                    file->cur_cluster = next;
                }
            }
        }
        // max space in block
        uint16_t n = 512 - block_offset;

        // lesser of space and amount to write
        if (n > n_to_write) {
            n = n_to_write;
        }

        // block for data write
        uint32_t block = sd_volume_cluster_start_block(file->vol,
                                                       file->cur_cluster);
        block +=  block_of_cluster;

        if (n == 512) {
            // full block - don't need to use cache
            // invalidate cache if block is in cache
            if (cache_block_number == block) {
                cache_block_number = 0xFFFFFFFF;
            }
            if (!sd_card_write_block(sd_card, block, src)) {
                goto write_error_return;
            }
            src += 512;
        } else {
            if (block_offset == 0 && file->cur_position >= file->file_size) {
                // start of new block don't need to read into cache
                if (!sd_volume_cache_flush()) {
                    goto write_error_return;
                }
                cache_block_number = block;
                sd_volume_cache_set_dirty();
            } else {
                // rewrite part of block
                if (!sd_volume_cache_raw_block(block, CACHE_FOR_WRITE)) {
                    goto write_error_return;
                }
            }

            uint8_t* dst = cache_buffer.data + block_offset;
            uint8_t* end = dst + n;
            while (dst != end) {
                *dst++ = *src++;
            }
        }
        n_to_write -= n;
        file->cur_position += n;
    }
    if (file->cur_position > file->file_size) {
        // update file_size and insure sync will update dir entry
        file->file_size = file->cur_position;
        file->flags |= F_FILE_DIR_DIRTY;
    } else if (file->date_time && nbyte) {
        // insure sync will update modified date and time
        file->flags |= F_FILE_DIR_DIRTY;
    }

    if (file->flags & O_SYNC) {
        if (!sd_file_sync(file)) {
            goto write_error_return;
        }
    }
    return nbyte;

write_error_return:
    // return for write error
    file->write_error = 1;
    return 0;
}


size_t sd_file_send(SdFile* file, uint8_t b)
{
    return sd_file_write(file, &b, 1);
}


size_t sd_file_print(SdFile* file, const char* str)
{
    return sd_file_write(file, (const uint8_t*) str, strlen(str));
}

size_t sd_file_println(SdFile* file, const char* str)
{
    size_t size = sd_file_print(file, str);
    sd_file_crlf(file);
    return size + 2;
}


void sd_file_write_P(SdFile* file, PGM_P str)
{
    for (uint8_t c; (c = pgm_read_byte(str)); str++) {
        sd_file_send(file, c);
    }
}


void sd_file_writeln_P(SdFile* file, PGM_P str)
{
    sd_file_write_P(file, str);
    sd_file_crlf(file);
}
