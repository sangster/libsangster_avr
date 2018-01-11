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
 * SdVolume class
 *
 * This library aims to expose a subset of SD card functionality
 * in the form of a higher level "wrapper" object.
 *
 * License: GNU General Public License V3
 *          (Because sdfatlib is licensed with this.)
 *
 * (C) Copyright 2010 SparkFun Electronics
 */
#ifndef SD_VOLUME_H
#define SD_VOLUME_H
/**
 * @file
 */

#include <stdbool.h>
#include "sangster/sd/fat_structs.h"
#include "sangster/sd/sd_card.h"

// value for action argument in cache_raw_block to indicate read from cache
#define CACHE_FOR_READ 0

// value for action argument in cache_raw_block to indicate cache dirty
#define CACHE_FOR_WRITE 1


/**
 * @brief Cache for an SD data block
 */
union sd_cache
{
    uint8_t data[512];   /** Used to access cached file data blocks. */
    uint16_t fat16[256]; /** Used to access cached FAT16 entries. */
    uint32_t fat32[128]; /** Used to access cached FAT32 entries. */
    SdDir dir[16];       /** Used to access cached directory entries. */
    SdMbr mbr;           /** Used to access a cached MasterBoot Record. */
    SdFbs fbs;           /** Used to access to a cached FAT boot sector. */
};
typedef union sd_cache SdCache;

typedef struct sd_volume SdVolume;
struct sd_volume
{
    uint32_t alloc_search_start;    // start cluster for alloc search
    uint8_t blocks_per_cluster;     // cluster size in blocks
    uint32_t blocks_per_fat;        // FAT size in blocks
    uint32_t cluster_count;         // clusters in one FAT
    uint8_t cluster_size_shift;     // shift to convert cluster count to block count
    uint32_t data_start_block;      // first data block number
    uint8_t fat_count;              // number of FATs on volume
    uint32_t fat_start_block;       // start block for first FAT
    uint8_t fat_type;               // volume type (12, 16, or 32)
    uint16_t root_dir_entry_count;  // number of entries in FAT16 root dir
    uint32_t root_dir_start;        // root start block for FAT16, cluster for FAT32
};


// raw block cache
extern SdCache cache_buffer;         // 512 byte cache for device blocks
extern uint32_t cache_block_number;  // Logical number of block in the cache
extern SdCard* sd_card;              // sd_card object for cache
extern uint8_t cache_dirty;          // cache_flush() will write block if true
extern uint32_t cache_mirror_block;  // block number for mirror FAT


inline uint8_t sd_volume_is_eoc(SdVolume* vol, uint32_t cluster)
{
    return cluster >= (vol->fat_type == 16 ? FAT16EOC_MIN : FAT32EOC_MIN);
}


inline uint8_t sd_volume_block_of_cluster(SdVolume* vol, uint32_t pos)
{
    return (pos >> 9) & (vol->blocks_per_cluster - 1);
}


inline uint32_t sd_volume_cluster_start_block(SdVolume* vol, uint32_t cluster)
{
    return vol->data_start_block + ((cluster - 2) << vol->cluster_size_shift);
}


inline uint32_t sd_volume_block_number(SdVolume* vol, uint32_t cluster,
                                       uint32_t pos)
{
    return sd_volume_cluster_start_block(vol, cluster) +
        sd_volume_block_of_cluster(vol, pos);
}


uint8_t sd_volume_init(SdVolume* vol, SdCard* dev, uint8_t part);


uint8_t sd_volume_init_try_both(SdVolume* vol, SdCard* dev);


uint8_t sd_volume_cache_flush();


uint8_t sd_volume_chain_size(SdVolume* vol, uint32_t cluster, uint32_t* size);


uint8_t sd_volume_fat_get(SdVolume* vol, uint32_t cluster, uint32_t* value);


// free a cluster chain
uint8_t sd_volume_free_chain(SdVolume* vol, uint32_t cluster);


// Store a FAT entry
uint8_t sd_volume_fat_put(SdVolume* vol, uint32_t cluster, uint32_t value);


uint8_t sd_volume_fat_put_eoc(SdVolume* vol, uint32_t cluster);


uint8_t sd_volume_cache_raw_block(uint32_t block_number, uint8_t action);


void sd_volume_cache_set_dirty();


// cache a zero block for blockNumber
uint8_t sd_volume_cache_zero_block(uint32_t block_number);


// find a contiguous group of clusters
uint8_t sd_volume_alloc_contiguous(SdVolume* vol, uint32_t count,
                                   uint32_t* cur_cluster);

#endif//SD_VOLUME_H
