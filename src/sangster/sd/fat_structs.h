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
#ifndef FAT_STRUCTS_H
#define FAT_STRUCTS_H
/**
 * @file
 * FAT file structures
 *
 * mostly from Microsoft document fatgen103.doc
 * http://www.microsoft.com/whdc/system/platform/firmware/fatgen.mspx
 */
#include <stdint.h>


/** Value for byte 510 of boot block or MBR */
#define BOOTSIG0 0x55

/** Value for byte 511 of boot block or MBR */
#define BOOTSIG1 0xAA


/**
 * @struct partition_table
 * @brief MBR partition table entry
 *
 * A partition table entry for a MBR formatted storage device.
 * The MBR partition table has four entries.
 */
typedef struct partition_table SdPart;
struct partition_table
{
    /**
     * Boot Indicator. Indicates whether the volume is the active partition.
     * Legal values include: 0x00. Do not use for booting. 0x80 Active
     * partition.
     */
    uint8_t boot;

    /**
     * Head part of Cylinder-head-sector address of the first block in the
     * partition. Legal values are 0-255. Only used in old PC BIOS.
     */
    uint8_t begin_head;

    /**
     * Sector part of Cylinder-head-sector address of the first block in the
     * partition. Legal values are 1-63. Only used in old PC BIOS.
     */
    unsigned begin_sector : 6;

    /** High bits cylinder for first block in partition. */
    unsigned begin_cylinder_high : 2;

    /**
     * Combine begin_cylinder_low with begin_cylinder_high. Legal values are
     * 0-1023. Only used in old PC BIOS.
     */
    uint8_t begin_cylinder_low;

    /**
     * Partition type. See defines that begin with PART_TYPE_ for some
     * Microsoft partition types.
     */
    uint8_t type;

    /**
     * head part of cylinder-head-sector address of the last sector in the
     * partition. Legal values are 0-255. Only used in old PC BIOS.
     */
    uint8_t end_head;

    /**
     * Sector part of cylinder-head-sector address of the last sector in the
     * partition. Legal values are 1-63. Only used in old PC BIOS.
     */
    unsigned end_sector : 6;

    /** High bits of end cylinder */
    unsigned end_cylinder_high : 2;

    /**
     * Combine endCylinderLow with endCylinderHigh. Legal values are 0-1023.
     * Only used in old PC BIOS.
     */
    uint8_t end_cylinder_low;

    /** Logical block address of the first block in the partition. */
    uint32_t first_sector;

    /** Length of the partition, in blocks. */
    uint32_t total_sectors;
} __attribute__((packed));
//------------------------------------------------------------------------------


/**
 * @struct master_boot_record
 *
 * @brief Master Boot Record
 *
 * The first block of a storage device that is formatted with a MBR.
 */
typedef struct master_boot_record SdMbr;
struct master_boot_record
{
    /** Code Area for master boot program. */
    uint8_t code_area[440];

    /** Optional WindowsNT disk signature. May contain more boot code. */
    uint32_t disk_signature;

    /** Usually zero but may be more boot code. */
    uint16_t usually_zero;

    /** Partition tables. */
    SdPart part[4];

    /** First MBR signature byte. Must be 0x55 */
    uint8_t mbr_sig0;

    /** Second MBR signature byte. Must be 0xAA */
    uint8_t mbr_sig1;
} __attribute__((packed));


/**
 * @struct bios_parm_block
 *
 * @brief BIOS parameter block
 *
 *  The BIOS parameter block describes the physical layout of a FAT volume.
 */
typedef struct bios_parm_block SdBpb;
struct bios_parm_block {
    /**
     * Count of bytes per sector. This value may take on only the following
     * values: 512, 1024, 2048 or 4096
     */
    uint16_t bytes_per_sector;

    /**
     * Number of sectors per allocation unit. This value must be a power of 2
     * that is greater than 0. The legal values are 1, 2, 4, 8, 16, 32, 64, and
     * 128.
     */
    uint8_t  sectors_per_cluster;

    /** Number of sectors before the first FAT. This value must not be zero. */
    uint16_t reserved_sector_count;

    /*
     * The count of FAT data structures on the volume. This field should always
     * contain the value 2 for any FAT volume of any type.
     */
    uint8_t fat_count;

    /**
     * For FAT12 and FAT16 volumes, this field contains the count of 32-byte
     * directory entries in the root directory. For FAT32 volumes, this field
     * must be set to 0. For FAT12 and FAT16 volumes, this value should always
     * specify a count that when multiplied by 32 results in a multiple of
     * bytes_per_sector. FAT16 volumes should use the value 512.
     */
    uint16_t root_dir_entry_count;

    /**
     * This field is the old 16-bit total count of sectors on the volume. This
     * count includes the count of all sectors in all four regions of the
     * volume. This field can be 0; if it is 0, then totalSectors32 must be
     * non-zero. For FAT32 volumes, this field must be 0. For FAT12 and FAT16
     * volumes, this field contains the sector count, and totalSectors32 is 0
     * if the total sector count fits (is less than 0x10000).
     */
    uint16_t total_sectors16;

    /**
     * This dates back to the old MS-DOS 1.x media determination and is no
     * longer usually used for anything. 0xF8 is the standard value for fixed
     * (non-removable) media. For removable media, 0xF0 is frequently used.
     * Legal values are 0xF0 or 0xF8-0xFF.
     */
    uint8_t mediaType;

    /**
     * Count of sectors occupied by one FAT on FAT12/FAT16 volumes. On FAT32
     * volumes this field must be 0, and sectors_per_fat32 contains the FAT size
     * count.
     */
    uint16_t sectors_per_fat16;

    /** Sectors per track for interrupt 0x13. Not used otherwise. */
    uint16_t sectors_per_trtack;

    /** Number of heads for interrupt 0x13. Not used otherwise. */
    uint16_t head_count;

    /**
     * Count of hidden sectors preceding the partition that contains this FAT
     * volume. This field is generally only relevant for media visible on
     * interrupt 0x13.
     */
    uint32_t hiddden_sectors;

    /**
     * This field is the new 32-bit total count of sectors on the volume.  This
     * count includes the count of all sectors in all four regions of the
     * volume. This field can be 0; if it is 0, then total_sectors16 must be
     * non-zero.
     */
    uint32_t total_sectors32;

    /** Count of sectors occupied by one FAT on FAT32 volumes.  */
    uint32_t sectors_per_fat32;

    /**
     * This field is only defined for FAT32 media and does not exist on FAT12
     * and FAT16 media.
     * Bits 0-3  -- Zero-based number of active FAT.
     *              Only valid if mirroring is disabled.
     * Bits 4-6  -- Reserved.
     * Bit  7    -- 0 means the FAT is mirrored at runtime into all FATs.
     *           -- 1 means only one FAT is active; it is the one referenced in
     *              bits 0-3.
     * Bits 8-15 -- Reserved.
     */
    uint16_t fat32_flags;

    /**
     * FAT32 version. High byte is major revision number.  Low byte is minor
     * revision number. Only 0.0 define.
     */
    uint16_t fat32_version;

    /**
     * Cluster number of the first cluster of the root directory for FAT32.
     * This usually 2 but not required to be 2.
     */
    uint32_t fat32_root_cluster;

    /**
     * Sector number of FSINFO structure in the reserved area of the FAT32
     * volume. Usually 1.
     */
    uint16_t fat32_fs_info;

    /**
     * If non-zero, indicates the sector number in the reserved area of the
     * volume of a copy of the boot record. Usually 6.  No value other than 6
     * is recommended.
     */
    uint16_t fat32_back_boot_block;

    /**
     * Reserved for future expansion. Code that formats FAT32 volumes should
     * always set all of the bytes of this field to 0.
     */
    uint8_t fat32_reserved[12];
} __attribute__((packed));


/**
 * @struct fat32_boot_sector
 *
 * @brief Boot sector for a FAT16 or FAT32 volume.
 */
typedef struct fat32_boot_sector SdFbs;
struct fat32_boot_sector {
    /** X86 jmp to boot program */
    uint8_t jmp_to_boot_code[3];

    /** informational only - don't depend on it */
    char oem_name[8];

    /** BIOS Parameter Block */
    SdBpb bpb;

    /** for int0x13 use value 0x80 for hard drive */
    uint8_t drive_number;

    /** used by Windows NT - should be zero for FAT */
    uint8_t reserved1;

    /** 0x29 if next three fields are valid */
    uint8_t boot_signature;

    /** usually generated by combining date and time */
    uint32_t volume_serial_number;

    /** should match volume label in root dir */
    char volume_label[11];

    /** informational only - don't depend on it */
    char file_system_type[8];

    /** X86 boot code */
    uint8_t boot_code[420];

    /** must be 0x55 */
    uint8_t boot_sector_sig0;

    /** must be 0xAA */
    uint8_t boot_sector_sig1;
} __attribute__((packed));


// End Of Chain values for FAT entries
/** FAT16 end of chain value used by Microsoft. */
#define FAT16EOC ((uint16_t) 0xFFFF)

/** Minimum value for FAT16 EOC. Use to test for EOC. */
#define FAT16EOC_MIN ((uint16_t) 0xFFF8)

/** FAT32 end of chain value used by Microsoft. */
#define FAT32EOC ((uint32_t) 0x0FFFFFFF)

/** Minimum value for FAT32 EOC. Use to test for EOC. */
#define FAT32EOC_MIN ((uint32_t) 0x0FFFFFF8)

/** Mask a for FAT32 entry. Entries are 28 bits. */
#define FAT32MASK ((uint32_t) 0x0FFFFFFF)


/**
 * @struct directory_entry
 * @brief FAT short directory entry
 *
 * Short means short 8.3 name, not the entry size.
 *
 * Date Format. A FAT directory entry date stamp is a 16-bit field that is
 * basically a date relative to the MS-DOS epoch of 01/01/1980. Here is the
 * format (bit 0 is the LSB of the 16-bit word, bit 15 is the MSB of the 16-bit
 * word):
 *
 * Bits 9-15: Count of years from 1980, valid value range 0-127 inclusive
 * (1980-2107).
 *
 * Bits 5-8: Month of year, 1 = January, valid value range 1-12 inclusive.
 *
 * Bits 0-4: Day of month, valid value range 1-31 inclusive.
 *
 * Time Format. A FAT directory entry time stamp is a 16-bit field that has a
 * granularity of 2 seconds. Here is the format (bit 0 is the LSB of the 16-bit
 * word, bit 15 is the MSB of the 16-bit word).
 *
 * Bits 11-15: Hours, valid value range 0-23 inclusive.
 *
 * Bits 5-10: Minutes, valid value range 0-59 inclusive.
 *
 * Bits 0-4: 2-second count, valid value range 0-29 inclusive (0 - 58 seconds).
 *
 * The valid time range is from Midnight 00:00:00 to 23:59:58.
 */
typedef struct directory_entry SdDir;
struct directory_entry
{
    /**
     * Short 8.3 name.  The first eight bytes contain the file name with blank
     * fill.  The last three bytes contain the file extension with blank fill.
     */
    uint8_t name[11];

    /**
     * Entry attributes.  The upper two bits of the attribute byte are reserved
     * and should always be set to 0 when a file is created and never modified
     * or looked at after that. See defines that begin with DIR_ATT_.
     */
    uint8_t attributes;

    /**
     * Reserved for use by Windows NT. Set value to 0 when a file is created
     * and never modify or look at it after that.
     */
    uint8_t reserved_nt;

    /**
     * The granularity of the seconds part of creationTime is 2 seconds so this
     * field is a count of tenths of a second and its valid value range is
     * 0-199 inclusive. (WHG note - seems to be hundredths)
     */
    uint8_t  creation_time_tenths;

    /** Time file was created. */
    uint16_t creation_time;

    /** Date file was created. */
    uint16_t creation_date;

    /**
     * Last access date. Note that there is no last access time, only a date.
     * This is the date of last read or write. In the case of a write, this
     * should be set to the same date as lastWriteDate.
     */
    uint16_t last_access_date;

    /**
     * High word of this entry's first cluster number (always 0 for a FAT12 or
     * FAT16 volume).
     */
    uint16_t first_cluster_high;

    /** Time of last write. File creation is considered a write. */
    uint16_t last_write_time;

    /** Date of last write. File creation is considered a write. */
    uint16_t last_write_date;

    /** Low word of this entry's first cluster number. */
    uint16_t first_cluster_low;

    /** 32-bit unsigned holding this file's size in bytes. */
    uint32_t file_size;
} __attribute__((packed));

//------------------------------------------------------------------------------
// Definitions for directory entries
//

/** escape for name[0] = 0xE5 */
#define DIR_NAME_0xE5 0x05

/** name[0] value for entry that is free after being "deleted" */
#define DIR_NAME_DELETED 0xE5

/** name[0] value for entry that is free and no allocated entries follow */
#define DIR_NAME_FREE 0x00

/** file is read-only */
#define DIR_ATT_READ_ONLY 0x01

/** File should hidden in directory listings */
#define DIR_ATT_HIDDEN 0x02

/** Entry is for a system file */
#define DIR_ATT_SYSTEM 0x04

/** Directory entry contains the volume label */
#define DIR_ATT_VOLUME_ID 0x08

/** Entry is for a directory */
#define DIR_ATT_DIRECTORY 0x10

/** Old DOS archive bit for backup support */
#define DIR_ATT_ARCHIVE 0x20

/** Test value for long name entry. Test is
  (d->attributes & DIR_ATT_LONG_NAME_MASK) == DIR_ATT_LONG_NAME. */
#define DIR_ATT_LONG_NAME 0x0F

/** Test mask for long name entry */
#define DIR_ATT_LONG_NAME_MASK 0x3F

/** defined attribute bits */
#define DIR_ATT_DEFINED_BITS 0x3F

/** Directory entry is part of a long name */
SA_INLINE uint8_t DIR_IS_LONG_NAME(const SdDir* dir)
{
  return (dir->attributes & DIR_ATT_LONG_NAME_MASK) == DIR_ATT_LONG_NAME;
}

/** Mask for file/subdirectory tests */
#define DIR_ATT_FILE_TYPE_MASK (DIR_ATT_VOLUME_ID | DIR_ATT_DIRECTORY)

/** Directory entry is for a file */
SA_INLINE uint8_t DIR_IS_FILE(const SdDir* dir)
{
  return (dir->attributes & DIR_ATT_FILE_TYPE_MASK) == 0;
}

/** Directory entry is for a subdirectory */
SA_INLINE uint8_t DIR_IS_SUBDIR(const SdDir* dir)
{
  return (dir->attributes & DIR_ATT_FILE_TYPE_MASK) == DIR_ATT_DIRECTORY;
}

/** Directory entry is for a file or subdirectory */
SA_INLINE uint8_t DIR_IS_FILE_OR_SUBDIR(const SdDir* dir)
{
  return (dir->attributes & DIR_ATT_VOLUME_ID) == 0;
}


typedef struct cid SdCid;
struct cid
{
    // byte 0
    uint8_t mid;  // Manufacturer ID
    // byte 1-2
    char oid[2];  // OEM/Application ID
    // byte 3-7
    char pnm[5];  // Product name
    // byte 8
    unsigned prv_m : 4;  // Product revision n.m
    unsigned prv_n : 4;
    // byte 9-12
    uint32_t psn;  // Product serial number
    // byte 13
    unsigned mdt_year_high : 4;  // Manufacturing date
    unsigned reserved : 4;
    // byte 14
    unsigned mdt_month : 4;
    unsigned mdt_year_low :4;
    // byte 15
    unsigned always1 : 1;
    unsigned crc : 7;
};

// CSD for version 1.00 cards
typedef struct csd_v1 SdCsdV1;
struct csd_v1
{
    // byte 0
    unsigned reserved1 : 6;
    unsigned csd_ver : 2;
    // byte 1
    uint8_t taac;
    // byte 2
    uint8_t nsac;
    // byte 3
    uint8_t tran_speed;
    // byte 4
    uint8_t ccc_high;
    // byte 5
    unsigned read_bl_len : 4;
    unsigned ccc_low : 4;
    // byte 6
    unsigned c_size_high : 2;
    unsigned reserved2 : 2;
    unsigned dsr_imp : 1;
    unsigned read_blk_misalign :1;
    unsigned write_blk_misalign : 1;
    unsigned read_bl_partial : 1;
    // byte 7
    uint8_t c_size_mid;
    // byte 8
    unsigned vdd_r_curr_max : 3;
    unsigned vdd_r_curr_min : 3;
    unsigned c_size_low :2;
    // byte 9
    unsigned c_size_mult_high : 2;
    unsigned vdd_w_cur_max : 3;
    unsigned vdd_w_curr_min : 3;
    // byte 10
    unsigned sector_size_high : 6;
    unsigned erase_blk_en : 1;
    unsigned c_size_mult_low : 1;
    // byte 11
    unsigned wp_grp_size : 7;
    unsigned sector_size_low : 1;
    // byte 12
    unsigned write_bl_len_high : 2;
    unsigned r2w_factor : 3;
    unsigned reserved3 : 2;
    unsigned wp_grp_enable : 1;
    // byte 13
    unsigned reserved4 : 5;
    unsigned write_partial : 1;
    unsigned write_bl_len_low : 2;
    // byte 14
    unsigned reserved5: 2;
    unsigned file_format : 2;
    unsigned tmp_write_protect : 1;
    unsigned perm_write_protect : 1;
    unsigned copy : 1;
    unsigned file_format_grp : 1;
    // byte 15
    unsigned always1 : 1;
    unsigned crc : 7;
};

// CSD for version 2.00 cards
typedef struct csd_v2 SdCsdV2;
struct csd_v2
{
    // byte 0
    unsigned reserved1 : 6;
    unsigned csd_ver : 2;
    // byte 1
    uint8_t taac;
    // byte 2
    uint8_t nsac;
    // byte 3
    uint8_t tran_speed;
    // byte 4
    uint8_t ccc_high;
    // byte 5
    unsigned read_bl_len : 4;
    unsigned ccc_low : 4;
    // byte 6
    unsigned reserved2 : 4;
    unsigned dsr_imp : 1;
    unsigned read_blk_misalign :1;
    unsigned write_blk_misalign : 1;
    unsigned read_bl_partial : 1;
    // byte 7
    unsigned reserved3 : 2;
    unsigned c_size_high : 6;
    // byte 8
    uint8_t c_size_mid;
    // byte 9
    uint8_t c_size_low;
    // byte 10
    unsigned sector_size_high : 6;
    unsigned erase_blk_en : 1;
    unsigned reserved4 : 1;
    // byte 11
    unsigned wp_grp_size : 7;
    unsigned sector_size_low : 1;
    // byte 12
    unsigned write_bl_len_high : 2;
    unsigned r2w_factor : 3;
    unsigned reserved5 : 2;
    unsigned wp_grp_enable : 1;
    // byte 13
    unsigned reserved6 : 5;
    unsigned write_partial : 1;
    unsigned write_bl_len_low : 2;
    // byte 14
    unsigned reserved7: 2;
    unsigned file_format : 2;
    unsigned tmp_write_protect : 1;
    unsigned perm_write_protect : 1;
    unsigned copy : 1;
    unsigned file_format_grp : 1;
    // byte 15
    unsigned always1 : 1;
    unsigned crc : 7;
};

// union of old and new style CSD register
union csd
{
    SdCsdV1 v1;
    SdCsdV2 v2;
};
typedef union csd SdCsd;

#endif//FAT_STRUCTS_H
