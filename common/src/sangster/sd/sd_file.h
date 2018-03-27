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
extern SdFileDateTime sd_date_time_callback;

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


/**
 * Initialize a new SdFile object.
 */
__attribute__((always_inline))
static inline void sd_file_init(SdFile* file)
{
    file->type = FAT_FILE_TYPE_CLOSED;
    file->date_time = NULL;
    file->write_error = 0;
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
uint8_t sd_file_open_root(SdFile* file, SdVolume* vol);


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
uint8_t sd_file_open(SdFile* file, SdFile* dir, const char* file_name,
                     uint8_t oflag);


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
uint8_t sd_file_open_by_index(SdFile* file, SdFile* dir, uint16_t index,
                              uint8_t oflag);


/**
 *  Close a file and force cached data and directory information to be written
 *  to the storage device.
 *
 * @return The value one, true, is returned for success and the value zero,
 *   false, is returned for failure.  Reasons for failure include no file is
 *   open or an I/O error.
 */
uint8_t sd_file_close(SdFile* file);


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
void sd_file_ls(SdFile* file, uint8_t flags, uint8_t indent);


static inline void sd_file_rewind(SdFile* file)
{
    file->cur_position = 0;
    file->cur_cluster = 0;
}


// Read next directory entry into the cache
// Assumes file is correctly positioned
SdDir* sd_file_read_dir_cache(SdFile* file);

int16_t sd_file_read(SdFile* file, uint8_t* dst, uint16_t nbyte);


/**
 * Read the next byte from a file.
 *
 * @return For success read returns the next byte in the file as an int. If an
 *   error occurs or end of file is reached -1 is returned.
 */
int16_t sd_file_read_next(SdFile* file);


/** @return Unbuffered read flag */
static inline uint8_t sd_file_unbuffered_read(SdFile* file)
{
    return file->flags & F_FILE_UNBUFFERED_READ;
}


/** @return True if this is a SdFile for a directory else false. */
static inline uint8_t sd_file_is_dir(SdFile* file)
{
    return file->type >= FAT_FILE_TYPE_MIN_DIR;
}

/** @return True if this is a SdFile for a file else false. */
static inline uint8_t sd_file_is_file(SdFile* file)
{
    return file->type == FAT_FILE_TYPE_NORMAL;
}

/** @return True if this is a SdFile for an open file/directory else false. */
static inline uint8_t sd_file_is_open(SdFile* file)
{
    return file->type != FAT_FILE_TYPE_CLOSED;
}

/** @return True if this is a SdFile for a subdirectory else false. */
static inline uint8_t sd_file_is_subdir(SdFile* file)
{
    return file->type == FAT_FILE_TYPE_SUBDIR;
}

/** @return True if this is a SdFile for the root directory. */
static inline uint8_t sd_file_is_root(SdFile* file)
{
    return file->type == FAT_FILE_TYPE_ROOT16 || file->type == FAT_FILE_TYPE_ROOT32;
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
int16_t sd_file_read(SdFile*, uint8_t* dst, uint16_t nbyte);


/**
 * Print the name field of a directory entry in 8.3 format to Serial.
 *
 * @param[in] dir The directory structure containing the name.
 * @param[in] width Blank fill name if length is less than @a width.
 */
void sd_file_print_dir_name(const SdDir dir, uint8_t width);

/**
 * Print a directory date field to Serial.
 *
 * Format is yyyy-mm-dd.
 *
 * @param[in] fatDate The date field from a directory entry.
 */
void sd_file_print_fat_date(uint16_t fat_date);


/**
 * Print a directory time field to Serial.
 *
 * Format is hh:mm:ss.
 *
 * @param[in] fatTime The time field from a directory entry.
 */
void sd_file_print_fat_time(uint16_t fat_time);

// format directory name field from a 8.3 name string
uint8_t sd_file_make83_name(const char* str, uint8_t* name);

// open a cached directory entry. Assumes vol_ is initialized
uint8_t sd_file_open_cached_entry(SdFile*, uint8_t dir_index, uint8_t oflag);


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
uint8_t sd_file_truncate(SdFile*, uint32_t length);


/**
 * Sets a file's position.
 *
 * @param[in] pos The new position in bytes from the beginning of the file.
 *
 * @return The value one, true, is returned for success and the value zero,
 *   false, is returned for failure.
 */
uint8_t sd_file_seek_set(SdFile*, uint32_t pos);

/**
 * The sync() call causes all modified data and directory fields to be written
 * to the storage device.
 *
 * @return The value one, true, is returned for success and the value zero,
 *   false, is returned for failure. Reasons for failure include a call to
 *   sync() before a file has been opened or an I/O error.
 */
uint8_t sd_file_sync(SdFile*);


// Add a cluster to a directory file and zero the cluster.
// return with first block of cluster in the cache
uint8_t sd_file_add_dir_cluster(SdFile*);

// add a cluster to a file
uint8_t sd_file_add_cluster(SdFile*);

SdDir* sd_file_cache_dir_entry(SdFile*, uint8_t action);


/**
 * Return a files directory entry
 *
 * @param[out] dir Location for return of the files directory entry.
 *
 * @return The value one, true, is returned for success and the value zero,
 *   false, is returned for failure.
 */
uint8_t sd_file_dir_entry(SdFile*, SdDir* dir);


/**
 * Format the name field of @a dir into the 13 byte array @a name in standard
 * 8.3 short name format.
 *
 * @param[in] dir The directory structure containing the name.
 * @param[out] name A 13 byte char array for the formatted name.
 */
void sd_file_dir_name(const SdDir* dir, char* name);


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
uint8_t sd_file_make_dir(SdFile*, SdFile* dir, const char* dir_name);


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
int8_t sd_file_read_dir(SdFile*, SdDir* dir);


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
uint8_t sd_file_remove(SdFile*);


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
uint8_t sd_file_remove_path(SdFile*, const char* file_name);


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
uint8_t sd_file_rm_dir(SdFile*);


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
uint8_t sd_file_rm_rf(SdFile*);


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
uint8_t sd_file_timestamp(SdFile*, uint8_t flags, uint16_t year, uint8_t month,
                          uint8_t day, uint8_t hour, uint8_t minute,
                          uint8_t second);


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
size_t sd_file_write(SdFile*, const uint8_t*, uint16_t n);


/**
 * Write a byte to a file. Required by the Arduino Print class.
 *
 * Use SdFile.write_error to check for errors.
 */
size_t sd_file_send(SdFile*, uint8_t);

static inline size_t sd_file_crlf(SdFile* file)
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
size_t sd_file_print(SdFile*, const char*);


/**
 * Write a string to a file. Used by the Arduino Print class.
 *
 * Use SdFile.write_error to check for errors.
 */
size_t sd_file_println(SdFile*, const char*);


/**
 * Write a PROGMEM string to a file.
 *
 * Use SdFile::writeError to check for errors.
 */
void sd_file_write_P(SdFile*, PGM_P);


/**
 * Write a PROGMEM string followed by CR/LF to a file.
 *
 * Use SdFile::writeError to check for errors.
 */
void sd_file_writeln_P(SdFile*, PGM_P);


/** date field for FAT directory entry */
static inline uint16_t FAT_DATE(uint16_t year, uint8_t month, uint8_t day)
{
    return (year - 1980) << 9 | month << 5 | day;
}
/** year part of FAT directory date field */
static inline uint16_t FAT_YEAR(uint16_t fat_date)
{
    return 1980 + (fat_date >> 9);
}
/** month part of FAT directory date field */
static inline uint8_t FAT_MONTH(uint16_t fat_date)
{
    return (fat_date >> 5) & 0xF;
}
/** day part of FAT directory date field */
static inline uint8_t FAT_DAY(uint16_t fat_date)
{
    return fat_date & 0x1F;
}
/** time field for FAT directory entry */
static inline uint16_t FAT_TIME(uint8_t hour, uint8_t minute, uint8_t second)
{
    return hour << 11 | minute << 5 | second >> 1;
}
/** hour part of FAT directory time field */
static inline uint8_t FAT_HOUR(uint16_t fat_time)
{
    return fat_time >> 11;
}
/** minute part of FAT directory time field */
static inline uint8_t FAT_MINUTE(uint16_t fat_time)
{
    return (fat_time >> 5) & 0x3F;
}
/** second part of FAT directory time field */
static inline uint8_t FAT_SECOND(uint16_t fat_time)
{
    return 2 * (fat_time & 0x1F);
}

#endif//SD_FILE_H
