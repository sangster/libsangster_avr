/*
 *  "libsangster_avr_common" is a library of common AVR functionality.
 *  Copyright (C) 2018  Jon Sangster
 *
 *  This program is free software: you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License as published by the Free
 *  Software Foundation, either version 3 of the License, or (at your option)
 *  any later version.
 *
 *  This program is distributed in the hope that it will be useful, but WITHOUT
 *  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 *  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 *  more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#include "util.h"

uint16_t ipow(uint8_t base, uint8_t exp)
{
    uint16_t res = 1;

    while(exp) {
        if (exp & 0x01) {
            res *= base;
        }
        exp >>= 1;
        base *= base;
    }
    return res;
}


int8_t parse_digit(const uint8_t input)
{
    if (input >= '0' && input <= '9') {
        return input - '0';
    } else if (input == ' ') {
        return 0;
    }
    return -1;
}


uint16_t map_u16(uint16_t x, uint16_t in_min, uint16_t in_max,
                             uint16_t out_min, uint16_t out_max)
{
    return (((uint32_t) x) - in_min) * (out_max - out_min) /
           (in_max - in_min) + out_min;
}
