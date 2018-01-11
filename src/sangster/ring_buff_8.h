#ifndef SANGSTER_RING_BUFF_H
#define SANGSTER_RING_BUFF_H
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
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

/** Creates a buffer of the given size */
#define RING_BUFF_8_DEF(_name, _size)               \
    uint8_t __ring_buff_ ## _name ## _data[_size];  \
    RingBuff8 _name = {                             \
        .buff = __ring_buff_ ## _name ## _data,     \
        .size = _size                               \
    }


struct ring_buff_8
{
    uint8_t * const buff;
    size_t head;
    size_t tail;
    size_t size;
    size_t used;
};
typedef struct ring_buff_8 RingBuff8;


#define ring_buff_8_init ring_buff_8_reset
void ring_buff_8_reset(RingBuff8* buff);
void ring_buff_8_push(RingBuff8* buff, uint8_t data);
uint8_t ring_buff_8_pop(RingBuff8* buff);
bool ring_buff_8_is_empty(const RingBuff8* buff);
bool ring_buff_8_is_full(const RingBuff8* buff);
uint8_t ring_buff_8_avg(RingBuff8* buff);


__attribute__((always_inline))
static inline size_t ring_buff_8_size(const RingBuff8* buff)
{
    return buff->size;
}


__attribute__((always_inline))
static inline uint8_t* ring_buff_8_get(const RingBuff8* buff)
{
    return buff->buff;
}

#endif//SANGSTER_RING_BUFF_H
