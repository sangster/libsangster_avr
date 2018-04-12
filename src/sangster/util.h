#ifndef SANGSTER_UTIL_H
#define SANGSTER_UTIL_H
/*
 * "libsangster_avr" is a library of common AVR functionality.
 * Copyright (C) 2018  Jon Sangster
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */
/**
 * @file
 */
#include <stdint.h>
#include "sangster/api.h"


/*******************************************************************************
 * Definitions
 ******************************************************************************/
/// @return The number of clock cycles in the given number of milliseconds
#define cycles_per_ms(ms, prescaler) \
    ((ms) / ((prescaler) / (F_CPU / 1e3)) - 1)


/// @return The number of clock cycles in the given number of microseconds
#define cycles_per_us(us, prescaler) \
    ((us) / ((prescaler) / (F_CPU / 1e6)) - 1)


/// @return Duration of the given number of clock cycles, in milliseconds
#define ms_per_cycle(cycles, prescaler) \
    (((cycles) + 1) / ((prescaler) / (F_CPU / 1e3)))


/// @return Duration of the given number of clock cycles, in microseconds
#define us_per_cycle(cycles, prescaler) \
    (((cycles) + 1) / ((prescaler) / (F_CPU / 1e6)))


/*******************************************************************************
 * Function Declarations
 ******************************************************************************/
/// Integer version of the math "pow" function
SA_FUNC uint16_t ipow(uint8_t, uint8_t);

/*
 * @return The digit represented by the given character. `-1` if the given
 *   character is not a digit. A space counts as 0.
 */
SA_FUNC int8_t parse_digit(const uint8_t);

/// Map a uint16_t from one range, to another
SA_INLINE uint16_t map_u16(uint16_t x,
                           uint16_t in_min,  uint16_t in_max,
                           uint16_t out_min, uint16_t out_max);


/*******************************************************************************
 * Function Definitions
 ******************************************************************************/
SA_FUNC uint16_t ipow(uint8_t base, uint8_t exp)
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


SA_FUNC int8_t parse_digit(const uint8_t input)
{
    if (input >= '0' && input <= '9') {
        return input - '0';
    } else if (input == ' ') {
        return 0;
    }
    return -1;
}


SA_INLINE uint16_t map_u16(const uint16_t x,
                           const uint16_t in_min,  const uint16_t in_max,
                           const uint16_t out_min, const uint16_t out_max)
{
    return (((uint32_t) x) - in_min) * (out_max - out_min) /
           (in_max - in_min) + out_min;
}
#endif//SANGSTER_UTIL_H
