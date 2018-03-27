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
#include "sonar.h"

uint16_t sonar_ping(SonarState* sonar)
{
    timer0_reset();

    if (!ping_trigger(sonar)) {
        return 0;
    }
    uint16_t time_before = timer0_us();

    // wait for the echo
    while (pinout_is_clr(sonar->interrupt)) {
        if (timer0_us() > sonar->timeout) {
            return 0;
        }
    }
    uint16_t time_after = timer0_us();

    return time_after - time_before - PING_OVERHEAD;
}


// @return 1 if the ECHO has begun
uint8_t ping_trigger(SonarState* sonar)
{
    // 14 us cycle: off 4 us, then on for 10 us
    pinout_clr(sonar->trigger);
    _delay_us(4);
    pinout_set(sonar->trigger);
    _delay_us(10);
    pinout_clr(sonar->trigger);

    pinout_set(sonar->capture);   // capture on rising edge
    pinout_set(sonar->interrupt); // Clear capture flag
    pinout_set(sonar->overflow);

    // Sensor warm-up
    sonar->start_at = timer0_us();
    sonar->timeout = sonar->start_at + MAX_SENSOR_DELAY; // set timeout

    // wait for rising edge of ECHO
    while (pinout_is_clr(sonar->interrupt)) {
        if (timer0_us() > sonar->timeout) {
            return 0; // error?
        }
    }
    pinout_clr(sonar->capture);   // capture on falling edge
    pinout_set(sonar->interrupt); // Clear capture flag
    pinout_set(sonar->overflow);

    // ping successful; update timeout
    sonar->start_at = timer0_us();
    sonar->timeout = sonar->start_at + (sonar->max_distance_cm * US_ROUNDTRIP_CM
                                        + (US_ROUNDTRIP_CM / 2)); // set timeout
    return 1;
}
