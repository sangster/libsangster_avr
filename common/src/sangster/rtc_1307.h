#ifndef SANGSTER_RTC_1307_H
#define SANGSTER_RTC_1307_H
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
 */
#include <time.h>
#include "sangster/twi.h"

// Address defined by RTC-1307 datasheet, p.8
#define RTC_1307_ADDR (_BV(6) | _BV(5) | _BV(3))

#define SEC_CH    7 ///< CLOCK HALT bit in the SECONDS register
#define HOUR_PM   5 ///< AM/PM SELECTION bit in the HOURS register
#define HOUR_12   6 ///< 12 hour SELECTION bit in the HOURS register
#define CTRL_RS0  0 ///< RATE SELECT 0 bit in the CONTROL register
#define CTRL_RS1  1 ///< RATE SELECT 1 bit in the CONTROL register
#define CTRL_SQWE 2 ///< SQUARE WAVE ENABLE bit in the CONTROL register
#define CTRL_OUT  3 ///< OUTPUT CONTROL bit in the CONTROL register


struct rtc_1307
{
    Twi twi;
};
typedef struct rtc_1307 Rtc1307;


uint8_t rtc_init(Rtc1307* clock);
TwiBusWriteRes rtc_disable();
uint8_t rtc_is_running();
TwiBusWriteRes rtc_set(struct tm* user_time);
uint8_t rtc_read(struct tm* dest);
uint8_t rtc_read_8(uint8_t addr);
void rtc_read_registers(uint8_t* bytes);


__attribute__((always_inline))
static inline uint8_t dec2bcd(const uint8_t x)
{
    return (x / 10) << 4 | (x % 10);
}


__attribute__((always_inline))
static inline uint8_t bcd2dec(const uint8_t x)
{
    return (x >> 4) * 10 + (x & 0x0F);
}

#endif//SANGSTER_RTC_1307_H
