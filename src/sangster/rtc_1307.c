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
#include "rtc_1307.h"

uint8_t rtc_init(Rtc1307* clock)
{
    twi_init(&clock->twi);
    return 1;
}


TwiBusWriteRes rtc_disable()
{
    twi_begin_tx(RTC_1307_ADDR);
    twi_write(0x00); // location: 0
    twi_write(_BV(7));
    return twi_end_tx(1);
}


uint8_t rtc_is_running()
{
    twi_begin_tx(RTC_1307_ADDR);
    twi_write(0x00);
    twi_end_tx(1);

    twi_bus_request(RTC_1307_ADDR, 1, 0, 0, 1);

    return !(twi_read() & _BV(SEC_CH));
}


TwiBusWriteRes rtc_set(struct tm* user_time)
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
uint8_t rtc_read(struct tm* dest)
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


uint8_t rtc_read_8(uint8_t addr)
{
    twi_begin_tx(RTC_1307_ADDR);
    twi_write(addr);
    twi_end_tx(1);
    return bcd2dec(twi_read());
}


void rtc_read_registers(uint8_t* bytes)
{
    twi_bus_request(RTC_1307_ADDR, 8, 0x00, 1, 1);

    for (int i = 0; i < 8; ++i) {
        bytes[i] = twi_read();
    }
}
