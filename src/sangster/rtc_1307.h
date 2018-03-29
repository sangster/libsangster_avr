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
#include "sangster/api.h"
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


typedef struct rtc_1307 Rtc1307;
struct rtc_1307
{
    Twi twi;
};


SA_INLINE uint8_t rtc_init(Rtc1307* clock)
{
    twi_init(&clock->twi);
    return 1;
}


SA_FUNC TwiBusWriteRes rtc_disable()
{
    twi_begin_tx(RTC_1307_ADDR);
    twi_write(0x00); // location: 0
    twi_write(_BV(7));
    return twi_end_tx(1);
}


SA_FUNC uint8_t rtc_is_running()
{
    twi_begin_tx(RTC_1307_ADDR);
    twi_write(0x00);
    twi_end_tx(1);

    twi_bus_request(RTC_1307_ADDR, 1, 0, 0, 1);

    return !(twi_read() & _BV(SEC_CH));
}


SA_INLINE uint8_t dec2bcd(const uint8_t x)
{
    return (x / 10) << 4 | (x % 10);
}


SA_INLINE uint8_t bcd2dec(const uint8_t x)
{
    return (x >> 4) * 10 + (x & 0x0F);
}


SA_FUNC TwiBusWriteRes rtc_set(struct tm* user_time)
{
    twi_begin_tx(RTC_1307_ADDR);
    twi_write(0x00); // location: 0
    twi_write(dec2bcd(user_time->tm_sec) & ~_BV(SEC_CH)); // enable oscillator
    twi_write(dec2bcd(user_time->tm_min));
    twi_write(dec2bcd(user_time->tm_hour) & ~_BV(HOUR_12)); // 24-hr mode
    twi_write(dec2bcd(user_time->tm_wday + 1));
    twi_write(dec2bcd(user_time->tm_mday));
    twi_write(dec2bcd(user_time->tm_mon + 1));
    twi_write(dec2bcd(user_time->tm_year - 100));
    return twi_end_tx(1);
}


/**
 * @return 1 if successful, 0 if not enough bytes read
 */
SA_FUNC uint8_t rtc_read(struct tm* dest)
{
    uint8_t size = twi_bus_request(RTC_1307_ADDR, 7, 0x00, 1, 1);
    if (size != 7) {
        return 0;
    }

    dest->tm_sec  = bcd2dec(twi_read() & ~_BV(SEC_CH));
    dest->tm_min  = bcd2dec(twi_read());
    dest->tm_hour = bcd2dec(twi_read() & ~_BV(HOUR_12));
    dest->tm_wday = bcd2dec(twi_read()) - 1;
    dest->tm_mday = bcd2dec(twi_read());
    dest->tm_mon  = bcd2dec(twi_read()) - 1;
    dest->tm_year = bcd2dec(twi_read()) + 100;

    dest->tm_isdst = -1;
    mktime(dest);

    return 1;
}


SA_FUNC uint8_t rtc_read_8(uint8_t addr)
{
    twi_begin_tx(RTC_1307_ADDR);
    twi_write(addr);
    twi_end_tx(1);
    return bcd2dec(twi_read());
}


SA_FUNC void rtc_read_registers(uint8_t* bytes)
{
    twi_bus_request(RTC_1307_ADDR, 8, 0x00, 1, 1);

    for (int i = 0; i < 8; ++i) {
        bytes[i] = twi_read();
    }
}
#endif//SANGSTER_RTC_1307_H
