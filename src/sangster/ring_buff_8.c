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
#include "ring_buff_8.h"

__attribute__((always_inline))
static inline bool tail_at_head(const RingBuff8* buff)
{
    return buff->tail == buff->head;
}


void ring_buff_8_reset(RingBuff8* buff)
{
    buff->head = 0;
    buff->tail = 0;
    buff->used = 0;
}


void ring_buff_8_push(RingBuff8* buff, uint8_t data)
{
    buff->buff[buff->head] = data;
    buff->head = (buff->head + 1) % buff->size;

    if (++buff->used > buff->size) {
        buff->used = buff->size;
        buff->tail = (buff->tail + 1) % buff->size;
    }
}


uint8_t ring_buff_8_pop(RingBuff8* buff)
{
    const uint8_t val = buff->buff[buff->tail];
    buff->tail = (buff->tail + 1) % buff->size;
    if (buff->used > 0) {
        --buff->used;
    }
    return val;
}


bool ring_buff_8_is_empty(const RingBuff8* buff)
{
    return buff->used == 0;
}


bool ring_buff_8_is_full(const RingBuff8* buff)
{
    return buff->used == buff->size;
}


uint8_t ring_buff_8_avg(RingBuff8* buff)
{
    uint16_t sum = 0;
    for (size_t i = 0; i < buff->used; ++i) {
        sum += buff->buff[(buff->tail + i) % buff->size];
    }
    return buff->used == 0 ? 0 : sum / buff->used;
}
