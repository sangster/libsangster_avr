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


/**
 * Print a value as two digits to Serial.
 *
 * @param[in] v Value to be printed, 0 <= \a v <= 99
 */
static void print_two_digits(uint8_t v) {
    char str[3];
    str[0] = '0' + v / 10;
    str[1] = '0' + v % 10;
    str[2] = 0;
    usart_print(str);
}


void sd_file_print_fat_date(uint16_t fat_date)
{
    usart_16(FAT_YEAR(fat_date));
    usart_send('-');
    print_two_digits(FAT_MONTH(fat_date));
    usart_send('-');
    print_two_digits(FAT_DAY(fat_date));
}


void sd_file_print_fat_time(uint16_t fat_time)
{
    print_two_digits(FAT_HOUR(fat_time));
    usart_send(':');
    print_two_digits(FAT_MINUTE(fat_time));
    usart_send(':');
    print_two_digits(FAT_SECOND(fat_time));
}


uint8_t sd_file_timestamp(SdFile* file, uint8_t flags, uint16_t year,
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
