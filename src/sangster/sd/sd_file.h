/* Arduino SdFat Library
 * Copyright (C) 2009 by William Greiman
 *
 * This file is part of the Arduino SdFat Library
 *
 * This Library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This Library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with the Arduino SdFat Library. If not, see
 * <http://www.gnu.org/licenses/>.
 */
#ifndef SD_FILE_H
#define SD_FILE_H
/**
 * @file
 */

#include <stdbool.h>
#include <string.h>
#include <avr/pgmspace.h>
#include "sangster/api.h"
#include "sangster/usart.h"
#include "sangster/sd/fat_structs.h"
#include "sangster/sd/sd_volume.h"


// flags for ls()
/** ls() flag to print modify date */
#define LS_DATE 1

/** ls() flag to print file size */
#define LS_SIZE 2

/** ls() flag for recursive list of subdirectories */
#define LS_R 4


// use the gnu style oflag in open()
/** open() oflag for reading */
#define O_READ 0x01

/** open() oflag - same as O_READ */
#define O_RDONLY O_READ

/** open() oflag for write */
#define O_WRITE 0x02

/** open() oflag - same as O_WRITE */
#define O_WRONLY O_WRITE

/** open() oflag for reading and writing */
#define O_RDWR (O_READ | O_WRITE)

/** open() oflag mask for access modes */
#define O_ACCMODE (O_READ | O_WRITE)

/** The file offset shall be set to the end of the file prior to each write. */
#define O_APPEND 0x04

/** synchronous writes - call sync() after each write */
#define O_SYNC 0x08

/** create the file if nonexistent */
#define O_CREAT 0x10

/** If O_CREAT and O_EXCL are set, open() shall fail if the file exists */
#define O_EXCL 0x20

/** truncate the file to zero length */
#define O_TRUNC 0x40


// flags for timestamp
/** set the file's last access date */
#define T_ACCESS 1

/** set the file's creation date and time */
#define T_CREATE 2

/** Set the file's write date and time */
#define T_WRITE 4


// values for type_
/** This SdFile has not been opened. */
#define FAT_FILE_TYPE_CLOSED 0

/** SdFile for a file */
#define FAT_FILE_TYPE_NORMAL 1

/** SdFile for a FAT16 root directory */
#define FAT_FILE_TYPE_ROOT16 2

/** SdFile for a FAT32 root directory */
#define FAT_FILE_TYPE_ROOT32 3

/** SdFile for a subdirectory */
#define FAT_FILE_TYPE_SUBDIR 4

/** Test value for directory type */
#define FAT_FILE_TYPE_MIN_DIR FAT_FILE_TYPE_ROOT16


/** Default date for file timestamps is 1 Jan 2000 */
#define FAT_DEFAULT_DATE ((uint16_t) ((2000 - 1980) << 9) | (1 << 5) | 1);

/** Default time for file timestamp is 1 am */
#define FAT_DEFAULT_TIME ((uint16_t)_BV(11));

// bits defined in flags
// should be 0xF
#define F_OFLAG (O_ACCMODE | O_APPEND | O_SYNC)
// available bits
#define F_UNUSED 0x30
// use unbuffered SD read
#define F_FILE_UNBUFFERED_READ 0x40
// sync of directory entry required
#define F_FILE_DIR_DIRTY 0x80

typedef void (*SdFileDateTime)(uint16_t* date, uint16_t* time);
SdFileDateTime sd_date_time_callback;

/**
 * @class sd_file
 * @brief Access FAT16 and FAT32 files on SD and SDHC cards.
 */
typedef struct sd_file SdFile;
struct sd_file
{

    uint8_t  flags;          // See above for definition of flags_ bits
    uint8_t  type;           // type of file see above for values
    uint32_t cur_cluster;    // cluster for current file position
    uint32_t cur_position;   // current file position in bytes from beginning
    uint32_t dir_block;      // SD block that contains directory entry for file
    uint8_t  dir_index;      // index of entry in dir_block 0 <= dir_index <= 0xF
    uint32_t file_size;      // file size in bytes
    uint32_t first_cluster;  // first cluster of file
    SdVolume* vol;           // volume where file is located
    uint8_t write_error;

    SdFileDateTime date_time;
};


/** Initialize a new SdFile object.  */
SA_INLINE void sd_file_init(SdFile* file)
{
    file->type = FAT_FILE_TYPE_CLOSED;
    file->date_time = NULL;
    file->write_error = 0;
}


/** date field for FAT directory entry */
SA_INLINE uint16_t FAT_DATE(uint16_t year, uint8_t month, uint8_t day)
{
    return (year - 1980) << 9 | month << 5 | day;
}


/** year part of FAT directory date field */
SA_INLINE uint16_t FAT_YEAR(uint16_t fat_date)
{
    return 1980 + (fat_date >> 9);
}


/** month part of FAT directory date field */
SA_INLINE uint8_t FAT_MONTH(uint16_t fat_date)
{
    return (fat_date >> 5) & 0xF;
}


/** day part of FAT directory date field */
SA_INLINE uint8_t FAT_DAY(uint16_t fat_date)
{
    return fat_date & 0x1F;
}


/** time field for FAT directory entry */
SA_INLINE uint16_t FAT_TIME(uint8_t hour, uint8_t minute, uint8_t second)
{
    return hour << 11 | minute << 5 | second >> 1;
}


/** hour part of FAT directory time field */
SA_INLINE uint8_t FAT_HOUR(uint16_t fat_time)
{
    return fat_time >> 11;
}


/** minute part of FAT directory time field */
SA_INLINE uint8_t FAT_MINUTE(uint16_t fat_time)
{
    return (fat_time >> 5) & 0x3F;
}


/** second part of FAT directory time field */
SA_INLINE uint8_t FAT_SECOND(uint16_t fat_time)
{
    return 2 * (fat_time & 0x1F);
}


/** @return Unbuffered read flag */
SA_INLINE uint8_t sd_file_unbuffered_read(SdFile* file)
{
    return file->flags & F_FILE_UNBUFFERED_READ;
}


/** @return True if this is a SdFile for a directory else false. */
SA_INLINE uint8_t sd_file_is_dir(SdFile* file)
{
    return file->type >= FAT_FILE_TYPE_MIN_DIR;
}

/** @return True if this is a SdFile for a file else false. */
SA_INLINE uint8_t sd_file_is_file(SdFile* file)
{
    return file->type == FAT_FILE_TYPE_NORMAL;
}

/** @return True if this is a SdFile for an open file/directory else false. */
SA_INLINE uint8_t sd_file_is_open(SdFile* file)
{
    return file->type != FAT_FILE_TYPE_CLOSED;
}

/** @return True if this is a SdFile for a subdirectory else false. */
SA_INLINE uint8_t sd_file_is_subdir(SdFile* file)
{
    return file->type == FAT_FILE_TYPE_SUBDIR;
}

/** @return True if this is a SdFile for the root directory. */
SA_INLINE uint8_t sd_file_is_root(SdFile* file)
{
    return file->type == FAT_FILE_TYPE_ROOT16 || file->type == FAT_FILE_TYPE_ROOT32;
}


/**
 * Open a volume's root directory.
 *
 * @param[in] vol The FAT volume containing the root directory to be opened.
 *
 * @return The value one, true, is returned for success and the value zero,
 *   false, is returned for failure. Reasons for failure include the FAT volume
 *   has not been initialized or it a FAT12 volume.
 */
SA_FUNC uint8_t sd_file_open_root(SdFile* file, SdVolume* vol)
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


SA_FUNC SdDir* sd_file_cache_dir_entry(SdFile* file, uint8_t action)
{
    if (!sd_volume_cache_raw_block(file->dir_block, action)) {
        return NULL;
    }
    return cache_buffer.dir + file->dir_index;
}


// add a cluster to a file
SA_FUNC uint8_t sd_file_add_cluster(SdFile* file)
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


// Add a cluster to a directory file and zero the cluster.
// return with first block of cluster in the cache
SA_FUNC uint8_t sd_file_add_dir_cluster(SdFile* file)
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


/**
 * The sync() call causes all modified data and directory fields to be written
 * to the storage device.
 *
 * @return The value one, true, is returned for success and the value zero,
 *   false, is returned for failure. Reasons for failure include a call to
 *   sync() before a file has been opened or an I/O error.
 */
SA_FUNC uint8_t sd_file_sync(SdFile* file)
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


/**
 * Sets a file's position.
 *
 * @param[in] pos The new position in bytes from the beginning of the file.
 *
 * @return The value one, true, is returned for success and the value zero,
 *   false, is returned for failure.
 */
SA_FUNC uint8_t sd_file_seek_set(SdFile* file, uint32_t pos)
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


/**
 * Truncate a file to a specified length. The current file position will be
 * maintained if it is less than or equal to @a length otherwise it will be set
 * to end of file.
 *
 * @param[in] length The desired length for the file.
 *
 * @return The value one, true, is returned for success and the value zero,
 *   false, is returned for failure. Reasons for failure include file is read
 *   only, file is a directory, @a length is greater than the current file size
 *   or an I/O error occurs.
 */
SA_FUNC uint8_t sd_file_truncate(SdFile* file, uint32_t length)
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


// open a cached directory entry. Assumes vol_ is initialized
SA_FUNC uint8_t sd_file_open_cached_entry(SdFile* file, uint8_t dir_index,
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


// format directory name field from a 8.3 name string
SA_FUNC uint8_t sd_file_make83_name(const char* str, uint8_t* name)
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


/**
 *  Close a file and force cached data and directory information to be written
 *  to the storage device.
 *
 * @return The value one, true, is returned for success and the value zero,
 *   false, is returned for failure.  Reasons for failure include no file is
 *   open or an I/O error.
 */
SA_FUNC uint8_t sd_file_close(SdFile* file)
{
    if(!sd_file_sync(file)) {
        return false;
    }
    file->type = FAT_FILE_TYPE_CLOSED;
    return true;
}


/**
 * Print a value as two digits to Serial.
 *
 * @param[in] v Value to be printed, 0 <= \a v <= 99
 */
SA_INLINE void print_two_digits(uint8_t v) {
    char str[3];
    str[0] = '0' + v / 10;
    str[1] = '0' + v % 10;
    str[2] = 0;
    usart_print(str);
}


/**
 * Print a directory time field to Serial.
 *
 * Format is hh:mm:ss.
 *
 * @param[in] fatTime The time field from a directory entry.
 */
SA_FUNC void sd_file_print_fat_time(uint16_t fat_time)
{
    print_two_digits(FAT_HOUR(fat_time));
    usart_send(':');
    print_two_digits(FAT_MINUTE(fat_time));
    usart_send(':');
    print_two_digits(FAT_SECOND(fat_time));
}


SA_INLINE void sd_file_rewind(SdFile* file)
{
    file->cur_position = 0;
    file->cur_cluster = 0;
}


/**
 * Read data from a file starting at the current position.
 *
 * @param[out] dst Pointer to the location that will receive the data.
 * @param[in] nbyte Maximum number of bytes to read.
 *
 * @return For success read() returns the number of bytes read. A value less
 *   than @a nbyte, including zero, will be returned if end of file is reached.
 *   If an error occurs, read() returns -1. Possible errors include read()
 *   called before a file has been opened, corrupt file system or an I/O error
 *   occurred.
 */
SA_FUNC int16_t sd_file_read(SdFile* file, uint8_t* dst, uint16_t nbyte)
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


/**
 * Read the next byte from a file.
 *
 * @return For success read returns the next byte in the file as an int. If an
 *   error occurs or end of file is reached -1 is returned.
 */
SA_INLINE int16_t sd_file_read_next(SdFile* file)
{
    uint8_t b;
    return sd_file_read(file, &b, 1) == 1 ? b : -1;
}


// Read next directory entry into the cache
// Assumes file is correctly positioned
SA_FUNC SdDir* sd_file_read_dir_cache(SdFile* file)
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


/**
 * Open a file by index.
 *
 * @param[in] dirFile An open SdFat instance for the directory.
 *
 * @param[in] index The @a index of the directory entry for the file to be
 *   opened. The value for @a index is (directory file position)/32.
 *
 * @param[in] oflag Values for @a oflag are constructed by a bitwise-inclusive
 *   OR of flags O_READ, O_WRITE, O_TRUNC, and O_SYNC.
 *
 * See open() by fileName for definition of flags and return values.
 */
SA_FUNC uint8_t sd_file_open_by_index(SdFile* file, SdFile* dir, uint16_t index,
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


/**
 * Open a file or directory by name.
 *
 * @param[in] dir An open SdFat instance for the directory containing the
 * file to be opened.
 *
 * @param[in] fileName A valid 8.3 DOS name for a file to be opened.
 *
 * @param[in] oflag Values for @a oflag are constructed by a bitwise-inclusive
 * OR of flags from the following list
 *
 * O_READ   - Open for reading.
 * O_RDONLY - Same as O_READ.
 * O_WRITE  - Open for writing.
 * O_WRONLY - Same as O_WRITE.
 * O_RDWR   - Open for reading and writing.
 * O_APPEND - If set, the file offset shall be set to the end of the
 *            file prior to each write.
 * O_CREAT  - If the file exists, this flag has no effect except as noted
 *            under O_EXCL below. Otherwise, the file shall be created
 * O_EXCL   - If O_CREAT and O_EXCL are set, open() shall fail if the file exists.
 * O_SYNC   - Call sync() after each write. This flag should not be used with
 *            write(uint8_t), write_P(PGM_P), writeln_P(PGM_P), or the Arduino
 *            Print class. These functions do character at a time writes so
 *            sync() will be called after each byte.
 * O_TRUNC  - If the file exists and is a regular file, and the file is
 *            successfully opened and is not read only, its length shall be
 *            truncated to 0.
 *
 * @note Directory files must be opened read only. Write and truncation is
 * not allowed for directory files.
 *
 * @return The value one, true, is returned for success and the value zero,
 *   false, is returned for failure. Reasons for failure include this SdFile is
 *   already open, @a dif_file is not a directory, @a file_name is invalid, the
 *   file does not exist or can't be opened in the access mode specified by
 *   oflag.
 */
SA_FUNC uint8_t sd_file_open(SdFile* file, SdFile* dir, const char* file_name,
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


/**
 * Print the name field of a directory entry in 8.3 format to Serial.
 *
 * @param[in] dir The directory structure containing the name.
 * @param[in] width Blank fill name if length is less than @a width.
 */
SA_FUNC void sd_file_print_dir_name(const SdDir dir, uint8_t width)
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


/**
 * Print a directory date field to Serial.
 *
 * Format is yyyy-mm-dd.
 *
 * @param[in] fatDate The date field from a directory entry.
 */
SA_FUNC void sd_file_print_fat_date(uint16_t fat_date)
{
    usart_16(FAT_YEAR(fat_date));
    usart_send('-');
    print_two_digits(FAT_MONTH(fat_date));
    usart_send('-');
    print_two_digits(FAT_DAY(fat_date));
}


/**
 * List directory contents to Serial.
 *
 * @param[in] flags The inclusive OR of
 *
 * LS_DATE - %Print file modification date
 * LS_SIZE - %Print file size.
 * LS_R    - Recursive list of subdirectories.
 *
 * @param[in] indent Amount of space before file name. Used for recursive
 * list to indicate subdirectory level.
 */
SA_FUNC void sd_file_ls(SdFile* file, uint8_t flags, uint8_t indent)
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


/**
 * Return a files directory entry
 *
 * @param[out] dir Location for return of the files directory entry.
 *
 * @return The value one, true, is returned for success and the value zero,
 *   false, is returned for failure.
 */
SA_FUNC uint8_t sd_file_dir_entry(SdFile* file, SdDir* dir)
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


/**
 * Format the name field of @a dir into the 13 byte array @a name in standard
 * 8.3 short name format.
 *
 * @param[in] dir The directory structure containing the name.
 * @param[out] name A 13 byte char array for the formatted name.
 */
SA_FUNC void sd_file_dir_name(const SdDir* dir, char* name)
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


/**
 * Make a new directory.
 *
 * @param[in] dir An open SdFat instance for the directory that will containing
 *   the new directory.
 *
 * @param[in] dir_name A valid 8.3 DOS name for the new directory.
 *
 * @return The value one, true, is returned for success and the value zero,
 *   false, is returned for failure.  Reasons for failure include this SdFile is
 *   already open, @a dir is not a directory, @a dir_name is invalid or already
 *   exists in @a dir.
 */
SA_FUNC uint8_t sd_file_make_dir(SdFile* file, SdFile* dir, const char* dir_name)
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


/**
 * Read the next directory entry from a directory file.
 *
 * @param[out] dir The SdDir struct that will receive the data.
 *
 * @return For success readDir() returns the number of bytes read.  A value of
 *   zero will be returned if end of file is reached.  If an error occurs,
 *   readDir() returns -1.  Possible errors include readDir() called before a
 *   directory has been opened, this is not a directory file or an I/O error
 *   occurred.
 */
SA_FUNC int8_t sd_file_read_dir(SdFile* file, SdDir* dir)
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


/**
 * Remove a file.
 *
 * The directory entry and all data for the file are deleted.
 *
 * @note This function should not be used to delete the 8.3 version of a
 *   file that has a long name. For example if a file has the long name
 *   "New Text Document.txt" you should not delete the 8.3 name "NEWTEX~1.TXT".
 *
 * @return The value one, true, is returned for success and the value zero,
 *   false, is returned for failure.  Reasons for failure include the file
 *   read-only, is a directory, or an I/O error occurred.
 */
SA_FUNC uint8_t sd_file_remove(SdFile* file)
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


/**
 * Remove a file by path.
 *
 * The directory entry and all data for the file are deleted.
 *
 * @param[in] The directory that contains the file.
 * @param[in] file_name The name of the file to be removed.
 *
 * @note This function should not be used to delete the 8.3 version of a
 *   file that has a long name. For example if a file has the long name
 *   "New Text Document.txt" you should not delete the 8.3 name "NEWTEX~1.TXT".
 *
 * @return The value one, true, is returned for success and the value zero,
 *   false, is returned for failure.  Reasons for failure include the file is a
 *   directory, is read only, @a dirFile is not a directory, \a fileName is not
 *   found or an I/O error occurred.
 */
SA_FUNC uint8_t sd_file_remove_path(SdFile* dir, const char* file_name)
{
  SdFile file;
  sd_file_init(&file);

  if (!sd_file_open(&file, dir, file_name, O_WRITE)) {
      return false;
  }
  return sd_file_remove(&file);
}


/**
 * Remove a directory file.
 *
 * The directory file will be removed only if it is empty and is not the root
 * directory.  rmDir() follows DOS and Windows and ignores the read-only
 * attribute for the directory.
 *
 * @note This function should not be used to delete the 8.3 version of a
 *   directory that has a long name. For example if a directory has the long
 *   name "New folder" you should not delete the 8.3 name "NEWFOL~1".
 *
 * @return The value one, true, is returned for success and the value zero,
 *   false, is returned for failure.  Reasons for failure include the file is
 *   not a directory, is the root directory, is not empty, or an I/O error
 *   occurred.
 */
SA_FUNC uint8_t sd_file_rm_dir(SdFile* file)
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


/**
 * Recursively delete a directory and all contained files.
 *
 * This is like the Unix/Linux 'rm -rf *' if called with the root directory
 * hence the name.
 *
 * Warning - This will remove all contents of the directory including
 * subdirectories.  The directory will then be removed if it is not root.  The
 * read-only attribute for files will be ignored.
 *
 * @note This function should not be used to delete the 8.3 version of a
 *   directory that has a long name.  See remove() and rmDir().
 *
 * @return The value one, true, is returned for success and the value zero,
 *   false, is returned for failure.
 */
SA_FUNC uint8_t sd_file_rm_rf(SdFile* file)
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


/**
 * Set a file's timestamps in its directory entry.
 *
 * @param[in] flags Values for \a flags are constructed by a bitwise-inclusive
 *   OR of flags from the following list
 *
 * T_ACCESS - Set the file's last access date.
 * T_CREATE - Set the file's creation date and time.
 * T_WRITE  - Set the file's last write/modification date and time.
 *
 * @param[in] year   Valid range 1980 - 2107 inclusive.
 * @param[in] month  Valid range 1 - 12 inclusive.
 * @param[in] day    Valid range 1 - 31 inclusive.
 * @param[in] hour   Valid range 0 - 23 inclusive.
 * @param[in] minute Valid range 0 - 59 inclusive.
 * @param[in] second Valid range 0 - 59 inclusive
 *
 * @note It is possible to set an invalid date since there is no check for the
 *   number of days in a month.
 *
 * @note Modify and access timestamps may be overwritten if a date time
 *   callback function has been set by dateTimeCallback().
 *
 * @return The value one, true, is returned for success and the value zero,
 *   false, is returned for failure.
 */
SA_FUNC uint8_t sd_file_timestamp(SdFile* file, uint8_t flags, uint16_t year,
                                 uint8_t month, uint8_t day, uint8_t hour,
                                 uint8_t minute, uint8_t second)
{
    if (!sd_file_is_open(file)
            || year < 1980 || year > 2107
            || month < 1   || month > 12
            || day < 1     || day > 31
            || hour > 23 || minute > 59 || second > 59) {
        return false;
    }

    SdDir* d = sd_file_cache_dir_entry(file, CACHE_FOR_WRITE);
    if (!d) {
        return false;
    }

    uint16_t dir_date = FAT_DATE(year, month, day);
    uint16_t dir_time = FAT_TIME(hour, minute, second);
    if (flags & T_ACCESS) {
        d->last_access_date = dir_date;
    }
    if (flags & T_CREATE) {
        d->creation_date = dir_date;
        d->creation_time = dir_time;
        // seems to be units of 1/100 second not 1/10 as Microsoft states
        d->creation_time_tenths = second & 1 ? 100 : 0;
    }
    if (flags & T_WRITE) {
        d->last_write_date = dir_date;
        d->last_write_time = dir_time;
    }

    sd_volume_cache_set_dirty();
    return sd_file_sync(file);
}


/**
 * Write data to an open file.
 *
 * @note Data is moved to the cache but may not be written to the storage
 *   device until sync() is called.
 *
 * @param[in] Pointer to the location of the data to be written.
 * @param[in] n Number of bytes to write.
 *
 * @return For success write() returns the number of bytes written, always \a
 *   nbyte. If an error occurs, write() returns -1. Possible errors include
 *   write() is called before a file has been opened, write is called for a
 *   read-only file, device is full, a corrupt file system or an I/O error.
 *
 */
SA_FUNC size_t sd_file_write(SdFile* file, const uint8_t* src, uint16_t nbyte)
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


/**
 * Write a byte to a file. Required by the Arduino Print class.
 *
 * Use SdFile.write_error to check for errors.
 */
SA_INLINE size_t sd_file_send(SdFile* file, uint8_t b)
{
    return sd_file_write(file, &b, 1);
}


SA_INLINE size_t sd_file_crlf(SdFile* file)
{
    sd_file_send(file, '\r');
    sd_file_send(file, '\n');
    return 2;
}


/**
 * Write a string followed by CR/LF to a file. Used by the Arduino Print class.
 *
 * Use SdFile.write_error to check for errors.
 */
SA_INLINE size_t sd_file_print(SdFile* file, const char* str)
{
    return sd_file_write(file, (const uint8_t*) str, strlen(str));
}


/**
 * Write a string to a file. Used by the Arduino Print class.
 *
 * Use SdFile.write_error to check for errors.
 */
SA_INLINE sd_file_println(SdFile* file, const char* str)
{
    size_t size = sd_file_print(file, str);
    sd_file_crlf(file);
    return size + 2;
}


/**
 * Write a PROGMEM string to a file.
 *
 * Use SdFile::writeError to check for errors.
 */
SA_FUNC void sd_file_write_P(SdFile* file, PGM_P str)
{
    for (uint8_t c; (c = pgm_read_byte(str)); str++) {
        sd_file_send(file, c);
    }
}


/**
 * Write a PROGMEM string followed by CR/LF to a file.
 *
 * Use SdFile::writeError to check for errors.
 */
SA_INLINE void sd_file_writeln_P(SdFile* file, PGM_P str)
{
    sd_file_write_P(file, str);
    sd_file_crlf(file);
}
#endif//SD_FILE_H
