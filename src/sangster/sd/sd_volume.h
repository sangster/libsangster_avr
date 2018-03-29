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
#include "sangster/api.h"
#include "sangster/sd/fat_structs.h"
#include "sangster/sd/sd_card.h"

// value for action argument in cache_raw_block to indicate read from cache
#define CACHE_FOR_READ 0

// value for action argument in cache_raw_block to indicate cache dirty
#define CACHE_FOR_WRITE 1


/**
 * @brief Cache for an SD data block
 */
typedef union sd_cache SdCache;
union sd_cache
{
    uint8_t data[512];   /** Used to access cached file data blocks. */
    uint16_t fat16[256]; /** Used to access cached FAT16 entries. */
    uint32_t fat32[128]; /** Used to access cached FAT32 entries. */
    SdDir dir[16];       /** Used to access cached directory entries. */
    SdMbr mbr;           /** Used to access a cached MasterBoot Record. */
    SdFbs fbs;           /** Used to access to a cached FAT boot sector. */
};

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
SdCache cache_buffer;         // 512 byte cache for device blocks
uint32_t cache_block_number;  // Logical number of block in the cache
SdCard* sd_card;              // sd_card object for cache
uint8_t cache_dirty;          // cache_flush() will write block if true
uint32_t cache_mirror_block;  // block number for mirror FAT


SA_INLINE uint8_t sd_volume_is_eoc(SdVolume* vol, uint32_t cluster)
{
    return cluster >= (vol->fat_type == 16 ? FAT16EOC_MIN : FAT32EOC_MIN);
}


SA_INLINE uint8_t sd_volume_block_of_cluster(SdVolume* vol, uint32_t pos)
{
    return (pos >> 9) & (vol->blocks_per_cluster - 1);
}


SA_INLINE uint32_t sd_volume_cluster_start_block(SdVolume* vol, uint32_t cluster)
{
    return vol->data_start_block + ((cluster - 2) << vol->cluster_size_shift);
}


SA_INLINE uint32_t sd_volume_block_number(SdVolume* vol, uint32_t cluster,
                                       uint32_t pos)
{
    return sd_volume_cluster_start_block(vol, cluster) +
        sd_volume_block_of_cluster(vol, pos);
}


SA_INLINE void sd_volume_cache_set_dirty()
{
    cache_dirty |= CACHE_FOR_WRITE;
}


SA_FUNC uint8_t sd_volume_cache_flush()
{
    if (cache_dirty) {
        if (!sd_card_write_block(sd_card, cache_block_number, cache_buffer.data)) {
            return false;
        }
        // mirror FAT tables
        if (cache_mirror_block) {
            if (!sd_card_write_block(sd_card, cache_mirror_block, cache_buffer.data)) {
                return false;
            }
            cache_mirror_block = 0;
        }
        cache_dirty = 0;
    }
    return true;
}


SA_FUNC uint8_t sd_volume_cache_raw_block(uint32_t block_number, uint8_t action)
{
    if (cache_block_number != block_number) {
        if (!sd_volume_cache_flush()) {
            return false;
        }
        if (!sd_card_read_block(sd_card, block_number, cache_buffer.data)) {
            return false;
        }
        cache_block_number = block_number;
    }
    cache_dirty |= action;
    return true;
}


/**
 * Initialize a FAT volume.
 *
 * @param[in] dev The SD card where the volume is located.
 *
 * @param[in] part The partition to be used. Legal values for \a part are 1-4
 *   to use the corresponding partition on a device formatted with a MBR, Master
 *   Boot Record, or zero if the device is formatted as a super floppy with the
 *   FAT boot sector in block zero.
 *
 * @return The value one, true, is returned for success and the value zero,
 *   false, is returned for failure. Reasons for failure include not finding a
 *   valid partition, not finding a valid FAT file system in the specified
 *   partition or an I/O error.
 */
SA_FUNC uint8_t sd_volume_init(SdVolume* vol, SdCard* dev, uint8_t part)
{
    uint32_t volume_start_block = 0;
    sd_card = dev;
    // if part == 0 assume super floppy with FAT boot sector in block zero
    // if part > 0 assume mbr volume with partition table
    if (part) {
        if (part > 4) {
            return false;
        }
        if (!sd_volume_cache_raw_block(volume_start_block, CACHE_FOR_READ)) {
            return false;
        }
        SdPart* p = &cache_buffer.mbr.part[part - 1];
        if ((p->boot & 0x7F) !=0  ||
                p->total_sectors < 100 ||
                p->first_sector == 0) {
            return false; // not a valid partition
        }
        volume_start_block = p->first_sector;
    }
    if (!sd_volume_cache_raw_block(volume_start_block, CACHE_FOR_READ)) {
        return false;
    }
    SdBpb* bpb = &cache_buffer.fbs.bpb;
    if (bpb->bytes_per_sector != 512 ||
            bpb->fat_count == 0 ||
            bpb->reserved_sector_count == 0 ||
            bpb->sectors_per_cluster == 0) {
        return false; // not valid FAT volume
    }
    vol->fat_count = bpb->fat_count;
    vol->blocks_per_cluster = bpb->sectors_per_cluster;

    // determine shift that is same as multiply by vol->blocks_per_cluster
    vol->cluster_size_shift = 0;
    while (vol->blocks_per_cluster != _BV(vol->cluster_size_shift)) {
        // error if not power of 2
        if (vol->cluster_size_shift++ > 7) {
            return false;
        }
    }
    vol->blocks_per_fat = bpb->sectors_per_fat16 ?
        bpb->sectors_per_fat16 : bpb->sectors_per_fat32;

    vol->fat_start_block = volume_start_block + bpb->reserved_sector_count;

    // count for FAT16 zero for FAT32
    vol->root_dir_entry_count = bpb->root_dir_entry_count;

    // directory start for FAT16 data_start for FAT32
    vol->root_dir_start =
        vol->fat_start_block + bpb->fat_count * vol->blocks_per_fat;

    // data start for FAT16 and FAT32
    vol->data_start_block =
        vol->root_dir_start + ((32 * bpb->root_dir_entry_count + 511) / 512);

    // total blocks for FAT16 or FAT32
    uint32_t total_blocks = bpb->total_sectors16 ?
        bpb->total_sectors16 : bpb->total_sectors32;
    // total data blocks
    vol->cluster_count = total_blocks - (vol->data_start_block - volume_start_block);

    // divide by cluster size to get cluster count
    vol->cluster_count >>= vol->cluster_size_shift;

    // FAT type is determined by cluster count
    if (vol->cluster_count < 4085) {
        vol->fat_type = 12;
    } else if (vol->cluster_count < 65525) {
        vol->fat_type = 16;
    } else {
        vol->root_dir_start = bpb->fat32_root_cluster;
        vol->fat_type = 32;
    }
    return true;
}


SA_INLINE uint8_t sd_volume_init_try_both(SdVolume* vol, SdCard* dev)
{
    return sd_volume_init(vol, dev, 1) ? true : sd_volume_init(vol, dev, 0);
}


// Fetch a FAT entry
SA_FUNC uint8_t sd_volume_fat_get(SdVolume* vol, uint32_t cluster, uint32_t* value)
{
    if (cluster > (vol->cluster_count + 1)) {
        return false;
    }
    uint32_t lba = vol->fat_start_block;
    lba += vol->fat_type == 16 ? cluster >> 8 : cluster >> 7;

    if (lba != cache_block_number) {
        if (!sd_volume_cache_raw_block(lba, CACHE_FOR_READ)) {
            return false;
        }
    }
    if (vol->fat_type == 16) {
        *value = cache_buffer.fat16[cluster & 0xFF];
    } else {
        *value = cache_buffer.fat32[cluster & 0x7F] & FAT32MASK;
    }
    return true;
}


// return the size in bytes of a cluster chain
SA_FUNC uint8_t sd_volume_chain_size(SdVolume* vol, uint32_t cluster, uint32_t* size)
{
    uint32_t s = 0;
    do {
        if (!sd_volume_fat_get(vol, cluster, &cluster)) {
            return false;
        }
        s += 512UL << vol->cluster_size_shift;
    } while (!sd_volume_is_eoc(vol, cluster));
    *size = s;
    return true;
}


// free a cluster chain
SA_FUNC uint8_t sd_volume_free_chain(SdVolume* vol, uint32_t cluster)
{
    vol->alloc_search_start = 2; // clear free cluster location

    do {
        uint32_t next;
        if (!sd_volume_fat_get(vol, cluster, &next)) {
            return false;
        }
        if (!sd_volume_fat_get(vol, cluster, 0)) { // free cluster
            return false;
        }
        cluster = next;
    } while (!sd_volume_is_eoc(vol, cluster));

    return true;
}


// Store a FAT entry
SA_FUNC uint8_t sd_volume_fat_put(SdVolume* vol, uint32_t cluster, uint32_t value)
{
    if (cluster < 2) { // error if reserved cluster
        return false;
    }

    // error if not in FAT
    if (cluster > (vol->cluster_count + 1)) {
        return false;
    }

    // calculate block address for entry
    uint32_t lba = vol->fat_start_block;
    lba += vol->fat_type == 16 ? cluster >> 8 : cluster >> 7;

    if (lba != cache_block_number) {
        if (!sd_volume_cache_raw_block(lba, CACHE_FOR_READ)) {
            return false;
        }
    }
    // store entry
    if (vol->fat_type == 16) {
        cache_buffer.fat16[cluster & 0xFF] = value;
    } else {
        cache_buffer.fat32[cluster & 0x7F] = value;
    }
    sd_volume_cache_set_dirty();

    // mirror second FAT
    if (vol->fat_count > 1) {
        cache_mirror_block = lba + vol->blocks_per_fat;
    }
    return true;
}


SA_FUNC inline uint8_t sd_volume_fat_put_eoc(SdVolume* vol, uint32_t cluster)
{
    return sd_volume_fat_put(vol, cluster, 0x0FFFFFFF);
}


// cache a zero block for blockNumber
SA_FUNC uint8_t sd_volume_cache_zero_block(uint32_t block_number)
{
    if (!sd_volume_cache_flush()) {
        return false;
    }

    // loop take less flash than memset(cacheBuffer_.data, 0, 512);
    for (uint16_t i = 0; i < 512; i++) {
        cache_buffer.data[i] = 0;
    }
    cache_block_number = block_number;
    sd_volume_cache_set_dirty();
    return true;
}


// find a contiguous group of clusters
SA_FUNC uint8_t sd_volume_alloc_contiguous(SdVolume* vol, uint32_t count,
                                          uint32_t* cur_cluster)
{
    uint32_t bgn_cluster; // start of group
    uint8_t set_start;    // flag to save place to start next search

    // set search start cluster
    if (*cur_cluster) {
        // try to make file contiguous
        bgn_cluster = *cur_cluster + 1;

        // don't save new start location
        set_start = false;
    } else {
        // start at likely place for free cluster
        bgn_cluster = vol->alloc_search_start;

        // save next search start if one cluster
        set_start = 1 == count;
    }

    uint32_t end_cluster = bgn_cluster;        // end of group
    uint32_t fat_end = vol->cluster_count + 1; // last cluster of FAT

    // search the FAT for free clusters
    for (uint32_t n = 0;; n++, end_cluster++) {
        // can't find space checked all clusters
        if (n >= vol->cluster_count) {
            return false;
        }

        // past end - start from beginning of FAT
        if (end_cluster > fat_end) {
            bgn_cluster = end_cluster = 2;
        }
        uint32_t f;
        if (!sd_volume_fat_get(vol, end_cluster, &f)) {
            return false;
        }

        if (f != 0) {
            // cluster in use try next cluster as bgnCluster
            bgn_cluster = end_cluster + 1;
        } else if ((end_cluster - bgn_cluster + 1) == count) {
            break; // done - found space
        }
    }
    // mark end of chain
    if (!sd_volume_fat_put_eoc(vol, end_cluster)) {
        return false;
    }

    // link clusters
    while (end_cluster > bgn_cluster) {
        if (!sd_volume_fat_put(vol, end_cluster - 1, end_cluster)) {
            return false;
        }
        end_cluster--;
    }
    if (*cur_cluster != 0) {
        // connect chains
        if (!sd_volume_fat_put(vol, *cur_cluster, bgn_cluster)) {
            return false;
        }
    }
    // return first cluster number to caller
    *cur_cluster = bgn_cluster;

    // remember possible next free cluster
    if (set_start) {
        vol->alloc_search_start = bgn_cluster + 1;
    }

    return true;
}
#endif//SD_VOLUME_H
