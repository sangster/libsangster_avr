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
#include "sangster/api.h"
#include "sangster/util.h"


#define UTIL_16_BIT_DIGIT_WIDTH 6

#ifndef BAUD_TOL
#define BAUD_TOL 2
#endif//BAUD_TOL


enum usart_frame_format
{
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
SA_FUNC void usart_init(const uint16_t baud, const UsartFrameFormat format)
{
    // TODO: is uint32_t too wide?
    const uint32_t baud_tolerance = 100 * baud + baud * BAUD_TOL;
    uint16_t ubrr_value = (F_CPU + 8UL * baud) / (16UL * baud) -1UL;
    uint8_t use_2x = 0;

    if (100 * F_CPU > (16 * (ubrr_value + 1)) * baud_tolerance) {
        use_2x = 1;
    } else if (100 * F_CPU < (16 * (ubrr_value + 1)) * baud_tolerance) {
        use_2x = 1;
    }

    if (use_2x) {
        ubrr_value = (F_CPU + 4UL * baud) / (8UL * baud) - 1UL;
        UCSR0A |= _BV(U2X0);
    } else {
        UCSR0A &= ~(_BV(U2X0));
    }

    UBRR0L = ubrr_value & 0xff;
    UBRR0H = ubrr_value >> 8;

    UCSR0C = format;
}


/** @return the next character received.  */
SA_INLINE uint8_t usart_recv()
{
    loop_until_bit_is_set(UCSR0A, RXC0);
    return UDR0;
}


/**
 * Prints a character.
 *
 * @param ch The character to transmit on serial line.
 */
SA_FUNC void usart_send(const uint8_t ch)
{
    loop_until_bit_is_set(UCSR0A, UDRE0);
    UDR0 = ch;
}


/**
 * Read at most @a n characters.
 *
 * @return the number of characters read
 */
SA_FUNC size_t usart_recvn(uint8_t* dst, size_t n, bool echo)
{
    for (size_t i = 0; i < n - 1; ++i) {
        dst[i] = usart_recv();
        if (dst[i] == '\r') {
            dst[i] = '\0';
            return i + 1;
        } else if (echo) {
            usart_send(dst[i]);
        }
    }
    dst[n - 1] = '\0';
    return n;
}


/** Prints a string to the serial connection.  */
SA_INLINE inline void usart_print(char const* ch)
{
    while (*ch) {
        usart_send((uint8_t) *ch++);
    }
}

/** Prints the string representation of the given `uint8_t`.  */
SA_FUNC void usart_8(const uint8_t num)
{
    char buff[4];
    utoa(num, buff, 10);
    usart_print(buff);
}


/** Prints the string representation of the given `uint16_t`.  */
SA_FUNC void usart_16(const uint16_t num)
{
    char buff[7];
    utoa(num, buff, 10);
    usart_print(buff);
}


/** Prints the string representation of the given `uint32_t`.  */
SA_FUNC void usart_32(const uint32_t num)
{
    char buff[11];
    ultoa(num, buff, 10);
    usart_print(buff);
}


/** Prints a hexidecimal string representation of the given `uint8_t`. */
SA_FUNC void usart_hex_8(const uint8_t num)
{
    char buff[5];
    sprintf(buff, "0x%02x", num);
    usart_print(buff);
}


/** Prints a hexidecimal string representation of the given `uint16_t`. */
SA_FUNC void usart_hex_16(const uint16_t num)
{
    char buff[7];
    sprintf(buff, "0x%04x", num);
    usart_print(buff);
}


/** Prints a hexidecimal string representation of the given `uint32_t`. */
SA_FUNC void usart_hex_32(const uint32_t num)
{
    char buff[11];
    sprintf(buff, "0x%08lx", num);
    usart_print(buff);
}

/** Prints a binary string representation of the given `uint8_t`.  */
SA_FUNC void usart_bin_8(const uint8_t num)
{
    uint8_t i = 8;
    while(i) {
        usart_send(num & _BV(--i) ? '1' : '0');
    }
}


/** Prints a binary string representation of the given `uint16_t`.  */
SA_FUNC void usart_bin_16(const uint16_t num)
{
    uint8_t i = 16;
    while(i) {
        usart_send(num & _BV(--i) ? '1' : '0');
    }
}


/** Prints a binary string representation of the given `uint32_t`.  */
SA_FUNC void usart_bin_32(const uint32_t num)
{
    uint8_t i = 32;
    while(i) {
        usart_send(num & _BV(--i) ? '1' : '0');
    }
}


/** Prints a `CRLF`.  */
SA_INLINE void usart_crlf()
{
    usart_send('\r');
    usart_send('\n');
}


/** Prints the given text, followed by a `CRLF`.  */
SA_INLINE void usart_println(char const* ch)
{
    usart_print(ch);
    usart_crlf();
}


SA_FUNC int usart_stream_recv(__attribute__((unused)) FILE* stream)
{
    return (int) usart_recv();
}


SA_FUNC int usart_stream_send(const char c, __attribute__((unused)) FILE* _stream)
{
    if (c == '\n') {
        usart_send((uint8_t) '\r');
    }
    usart_send((uint8_t) c);

    return 0;
}


/**
 * Configures `STDOUT` to write to USART.
 *
 * @note Using some standard `stdio.h` functions may seriously bloat your
 *   binary.
 */
SA_INLINE void usart_setup_stdout()
{
    stdout = fdevopen(usart_stream_send, NULL);
}

/**
 * Configures `STDIN` to read from USART.
 *
 * @note Using some standard `stdio.h` functions may seriously bloat your
 *   binary.
 */
SA_INLINE void usart_setup_stdin()
{
    stdin = fdevopen(NULL, usart_stream_recv);
}

/**
 * Configures `STDIN` and `STDOUT` to read and write to USART.
 *
 * @note Using some standard `stdio.h` functions may seriously bloat your
 *   binary.
 */
SA_INLINE void usart_setup_streams()
{
    usart_setup_stdin();
    usart_setup_stdout();
}


SA_INLINE uint8_t usart_is_recv_ready()
{
    return bit_is_set(UCSR0A, RXC0);
}


SA_FUNC uint16_t __usart_read_uint16(char const* prompt, uint8_t len,
                                     const uint16_t min, const uint16_t max,
                                     void (*prompt_print)(const char*))
{
    uint16_t res;
    uint8_t digits[UTIL_16_BIT_DIGIT_WIDTH], finish_early = 0, valid = 0, i;

    if (len > UTIL_16_BIT_DIGIT_WIDTH) {
        len = UTIL_16_BIT_DIGIT_WIDTH;
    }

    while (!valid) {
        memset(digits, 0, sizeof digits);
        res = 0;
        prompt_print(prompt);

        for (i = 0; i < len && !finish_early; ++i) {
            uint8_t ch = usart_recv();
            int8_t input = parse_digit(ch);
            usart_8(input);

            if (input == -1) {
                switch (ch) {
                    case '\b':
                    case 0x7F: // DEL
                        if (i > 0) {
                            usart_send('\b');
                            usart_send(' ');
                            usart_send('\b');
                            i--;
                        }
                        break;
                    case '\r':
                    case '\n':
                        finish_early = 1;
                        break;
                }
                i--;
                continue;
            }
            digits[i] = input;
        }

        for (len = i, i = 0; i < len; ++i) {
            res += digits[i] * ipow(10, len - i - 1);
        }

        if (res >= min && res <= max) {
            valid = 1;
        } else {
            usart_crlf();
            usart_16(res);
            prompt_print(PSTR(" must be between "));
            usart_16(min);
            prompt_print(PSTR(" and "));
            usart_16(max);
            prompt_print(PSTR(", inclusive."));
        }
    }
    return res;
}


/**
 * Prompt the user to enter a number, enforcing a min and max value.
 */
SA_FUNC uint16_t usart_read_uint16(char const* prompt, const uint8_t len,
                                  const uint16_t min, const uint16_t max)
{
    return __usart_read_uint16(prompt, len, min, max, usart_print);
}


/**
 * Print out the given array of bytes, in hex, binary, decimal, and (if
 * applicable) ASCII formats.
 */
SA_FUNC void usart_dump_array_8(uint8_t* arr, uint8_t len)
{
    char str[7];
    for(uint8_t i = 0; i < len; ++i) {
        sprintf(str, "0x%02x", i);
        usart_print(str);
        usart_print("  ");
        usart_bin_8(arr[i]);
        usart_send(' ');
        sprintf(str, "%3d", arr[i]);
        usart_print(str);

        if (isprint(arr[i])) {
            usart_send(' ');
            usart_send('\'');
            usart_send(arr[i]);
            usart_send('\'');
        }
        usart_crlf();
    }
}
#endif // SANGSTER_USART_H
