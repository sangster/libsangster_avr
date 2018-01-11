#ifndef SANGSTER_UTIL_H
#define SANGSTER_UTIL_H
/*
 *  "ATMega328/p Library" is a library of common ATmega328/p functionality.
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

/**
 * @file
 */
#include <stdint.h>


/** Integer version of the math "pow" function.  */
uint16_t ipow(uint8_t base, uint8_t exp);

/*
 * @return The digit represented by the given character. `-1` if the given
 *   character is not a digit. A space counts as 0.
 */
int8_t parse_digit(uint8_t ch);


/* Map a uint16_t from one range, to another */
uint16_t map_u16(uint16_t x, uint16_t in_min, uint16_t in_max,
                             uint16_t out_min, uint16_t out_max);


/** @return The number of clock cycles in the given number of milliseconds */
#define cycles_per_ms(ms, prescaler) \
    ((ms) / ((prescaler) / (F_CPU / 1e3)) - 1)


/** @return The number of clock cycles in the given number of microseconds */
#define cycles_per_us(us, prescaler) \
    ((us) / ((prescaler) / (F_CPU / 1e6)) - 1)


/** @return Duration of the given number of clock cycles, in milliseconds */
#define ms_per_cycle(cycles, prescaler) \
    (((cycles) + 1) / ((prescaler) / (F_CPU / 1e3)))


/** @return Duration of the given number of clock cycles, in microseconds */
#define us_per_cycle(cycles, prescaler) \
    (((cycles) + 1) / ((prescaler) / (F_CPU / 1e6)))

#endif // SANGSTER_UTIL_H
