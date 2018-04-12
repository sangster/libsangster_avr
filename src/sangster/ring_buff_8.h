#ifndef SANGSTER_RING_BUFF_H
#define SANGSTER_RING_BUFF_H
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
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include "sangster/api.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
/** Creates a buffer of the given size */
#define RING_BUFF_8_DEF(_name, _size)               \
    uint8_t __ring_buff_ ## _name ## _data[_size];  \
    RingBuff8 _name = {                             \
        .buff = __ring_buff_ ## _name ## _data,     \
        .size = _size                               \
    }


/*******************************************************************************
 * Types
 ******************************************************************************/
typedef struct ring_buff_8 RingBuff8;
struct ring_buff_8
{
    uint8_t * const buff;
    size_t head;
    size_t tail;
    size_t size;
    size_t used;
};


/*******************************************************************************
 * Function Declarations
 ******************************************************************************/
SA_INLINE bool tail_at_head(const RingBuff8*);

SA_FUNC void ring_buff_8_init(RingBuff8*);

SA_INLINE void ring_buff_8_reset(RingBuff8*);

SA_FUNC void ring_buff_8_push(RingBuff8*, uint8_t);

SA_FUNC uint8_t ring_buff_8_pop(RingBuff8*);

SA_INLINE bool ring_buff_8_is_empty(const RingBuff8*);

SA_INLINE bool ring_buff_8_is_full(const RingBuff8*);

SA_FUNC uint8_t ring_buff_8_avg(const RingBuff8*);

SA_INLINE size_t ring_buff_8_size(const RingBuff8*);

SA_INLINE uint8_t* ring_buff_8_get(const RingBuff8*);


/*******************************************************************************
 * Function Definitions
 ******************************************************************************/
SA_INLINE bool tail_at_head(const RingBuff8* buff)
{
    return buff->tail == buff->head;
}


SA_FUNC void ring_buff_8_init(RingBuff8* buff)
{
    buff->head = 0;
    buff->tail = 0;
    buff->used = 0;
}

SA_INLINE void ring_buff_8_reset(RingBuff8* buff)
{
    ring_buff_8_init(buff);
}


SA_FUNC void ring_buff_8_push(RingBuff8* buff, uint8_t data)
{
    buff->buff[buff->head] = data;
    buff->head = (buff->head + 1) % buff->size;

    if (++buff->used > buff->size) {
        buff->used = buff->size;
        buff->tail = (buff->tail + 1) % buff->size;
    }
}


SA_FUNC uint8_t ring_buff_8_pop(RingBuff8* buff)
{
    const uint8_t val = buff->buff[buff->tail];
    buff->tail = (buff->tail + 1) % buff->size;
    if (buff->used > 0) {
        --buff->used;
    }
    return val;
}


SA_INLINE bool ring_buff_8_is_empty(const RingBuff8* buff)
{
    return buff->used == 0;
}


SA_INLINE bool ring_buff_8_is_full(const RingBuff8* buff)
{
    return buff->used == buff->size;
}


SA_FUNC uint8_t ring_buff_8_avg(const RingBuff8* buff)
{
    uint16_t sum = 0;
    for (size_t i = 0; i < buff->used; ++i) {
        sum += buff->buff[(buff->tail + i) % buff->size];
    }
    return buff->used == 0 ? 0 : sum / buff->used;
}


SA_INLINE size_t ring_buff_8_size(const RingBuff8* buff)
{
    return buff->size;
}


SA_INLINE uint8_t* ring_buff_8_get(const RingBuff8* buff)
{
    return buff->buff;
}
#endif//SANGSTER_RING_BUFF_H
