#ifndef SANGSTER_USART_P_H
#define SANGSTER_USART_P_H
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
 *
 * PROGMEM versions of usart.h.
 */
#include <avr/pgmspace.h>
#include "sangster/api.h"
#include "sangster/usart.h"


/*******************************************************************************
 * Function Declarations
 ******************************************************************************/
/// Prints a string from PGM space to the serial connection
SA_INLINE void usart_print_P(PGM_P);

/// Prompt the user to enter a number, enforcing a min and max value
SA_INLINE uint16_t usart_read_uint16_P(PGM_P, uint8_t len,
                                       uint16_t min, uint16_t max);

/**
 * Prints a string from PGM space, followed by a CR/LF to the serial connection.
 */
SA_INLINE void usart_println_P(PGM_P);


/*******************************************************************************
 * Function Definitions
 ******************************************************************************/
SA_INLINE void usart_print_P(PGM_P ch)
{
    uint8_t byte;
    while ((byte = pgm_read_byte(ch++)) != '\0') {
        usart_send(byte);
    }
}


SA_INLINE uint16_t usart_read_uint16_P(PGM_P prompt, uint8_t len,
                                    const uint16_t min, const uint16_t max)
{
    return __usart_read_uint16(prompt, len, min, max, usart_print_P);
}


SA_INLINE void usart_println_P(PGM_P ch)
{
    usart_print_P(ch);
    usart_crlf();
}
#endif//SANGSTER_USART_P_H
