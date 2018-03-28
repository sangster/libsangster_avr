#ifndef SANGSTER_USART_H
#define SANGSTER_USART_H
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

/**
 * @file
 *
 * Utilities for communicated via the TX/RX pins using USART.
 */
#include <ctype.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include "sangster/util.h"


#define UTIL_16_BIT_DIGIT_WIDTH 6


enum usart_frame_format {
    /// 8 bit, No parity bit, 1 stop
    FORMAT_8N1 = (_BV(UCSZ00) | _BV(UCSZ01)) & ~(_BV(USBS0)),

    /// 8 bit, No parity bit, 2 stops
    FORMAT_8N2 = (_BV(UCSZ00) | _BV(UCSZ01) | _BV(USBS0))
};
typedef enum usart_frame_format UsartFrameFormat;

/**
 * Opens an IO connection to a serial device.
 *
 * @param baud The communcation speed, in symbols/sec. The serial device must
 *   be configured to use the same rate.
 * @param format The format of the connection. The serial device must be
 * configured with the same parameters.
 */
void usart_init(uint16_t baud, enum usart_frame_format format);

/** @return the next character received.  */
uint8_t usart_recv();

/**
 * Read at most @a n characters.
 *
 * @return the number of characters read
 */
size_t usart_recvn(uint8_t* dst, size_t n, bool echo);

/**
 * Prints a character.
 *
 * @param ch The character to transmit on serial line.
 */
void usart_send(uint8_t ch);

/** Prints a string to the serial connection.  */
__attribute__((always_inline))
static inline void usart_print(char const* ch)
{
    while (*ch) {
        usart_send((uint8_t) *ch++);
    }
}

/** Prints the string representation of the given `uint8_t`.  */
void usart_8(uint8_t num);

/** Prints the string representation of the given `uint16_t`.  */
void usart_16(uint16_t num);

/** Prints the string representation of the given `uint32_t`.  */
void usart_32(uint32_t num);

/** Prints a hexidecimal string representation of the given `uint8_t`.  */
void usart_hex_8(uint8_t num);

/** Prints a hexidecimal string representation of the given `uint16_t`.  */
void usart_hex_16(uint16_t num);

/** Prints a hexidecimal string representation of the given `uint32_t`.  */
void usart_hex_32(uint32_t num);

/** Prints a binary string representation of the given `uint8_t`.  */
void usart_bin_8(uint8_t num);

/** Prints a binary string representation of the given `uint16_t`.  */
void usart_bin_16(uint16_t num);

/** Prints a binary string representation of the given `uint32_t`.  */
void usart_bin_32(uint32_t num);


/** Prints a `CRLF`.  */
__attribute__((always_inline))
static inline void usart_crlf()
{
    usart_send('\r');
    usart_send('\n');
}


/** Prints the given text, followed by a `CRLF`.  */
__attribute__((always_inline))
static inline void usart_println(char const* ch)
{
    usart_print(ch);
    usart_crlf();
}

int usart_stream_recv(FILE* stream);

int usart_stream_send(char c, FILE* stream);

/**
 * Configures `STDOUT` to write to USART.
 *
 * @note Using some standard `stdio.h` functions may seriously bloat your
 *   binary.
 */
__attribute__((always_inline))
static inline void usart_setup_stdout()
{
    stdout = fdevopen(usart_stream_send, NULL);
}

/**
 * Configures `STDIN` to read from USART.
 *
 * @note Using some standard `stdio.h` functions may seriously bloat your
 *   binary.
 */
__attribute__((always_inline))
static inline void usart_setup_stdin()
{
    stdin = fdevopen(NULL, usart_stream_recv);
}

/**
 * Configures `STDIN` and `STDOUT` to read and write to USART.
 *
 * @note Using some standard `stdio.h` functions may seriously bloat your
 *   binary.
 */
__attribute__((always_inline))
static inline void usart_setup_streams() {
    usart_setup_stdin();
    usart_setup_stdout();
}


__attribute__((always_inline))
static inline uint8_t usart_is_recv_ready() {
    return bit_is_set(UCSR0A, RXC0);
}


uint16_t usart_read_uint16(char const* prompt, uint8_t len,
                           uint16_t min, uint16_t max);


uint16_t __usart_read_uint16(char const* prompt, uint8_t len,
                             uint16_t min, uint16_t max,
                             void (*prompt_print)(char const*));


/**
 * Print out the given array of bytes, in hex, binary, decimal, and (if
 * applicable) ASCII formats.
 */
void usart_dump_array_8(uint8_t* arr, uint8_t len);


#endif // SANGSTER_USART_H
