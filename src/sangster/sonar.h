#ifndef SANGSTER_SONAR_H
#define SANGSTER_SONAR_H
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
#include <stdint.h>
#include <avr/io.h>
#include <util/delay.h>
#include <sangster/timer0.h>
#include "sangster/api.h"
#include "sangster/pinout.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
/**
 * In addition to the speed of sound in air, the trigger function itself takes
 * some time to execute. This time must be included when calculating the
 * duration of the trigger/echo sequence.
 */
#define PING_OVERHEAD 5 // us

/// time it takes to travel 1 cm, in us (from sensor datasheet)
#define US_ROUNDTRIP_CM 58u

/// time it takes to travel 1", in us (from sensor datasheet)
#define US_ROUNDTRIP_IN 148u

/// max sensor start-up time, in us
#define MAX_SENSOR_DELAY 18000u

/// The number of clock cycles which ellapse per ms
#define TICKS_PER_MS(x, sc) ((x) * ((F_CPU) / ((sc) * 1000u)))

/// The number of clock cycles which ellapse per us
#define TICKS_PER_US(x, sc) ((x) * ((F_CPU) / ((sc) * 1000000u)))


/*******************************************************************************
 * Types
 ******************************************************************************/
typedef struct sonar_state SonarState;
struct sonar_state
{
    const Pinout trigger;   ///< The pin to trigger the ping
    const Pinout capture;   ///< Configures the echo interrupt
    const Pinout interrupt; ///< Clears the echo interrupt
    const Pinout overflow;  ///< Clears the interrupt overflow

    uint16_t max_distance_cm;
    uint16_t start_at;
    uint16_t timeout;
};


/*******************************************************************************
 * Function Declarations
 ******************************************************************************/
/**
 * Triggers the device to send out the ping and waits for its echo signal.
 *
 * @return `1` if the signal was successfully sent and the `echo` has begun.
 * `0` if the ping timed-out.
 */
SA_FUNC uint8_t ping_trigger(SonarState*);

/**
 * @return How long it took for the echo to return, in microseconds. `0` if no
 *   echo was received (likely the maximum distance was exceeded).
 */
SA_FUNC uint16_t sonar_ping(SonarState*);

/// @return The distance measured by the sensor, in centimeters
SA_INLINE sonar_ping_cm(SonarState*);

/// @return The distance measured by the sensor, in inches
SA_INLINE sonar_ping_in(SonarState*);


/*******************************************************************************
 * Function Definitions
 ******************************************************************************/
SA_FUNC uint8_t ping_trigger(SonarState* sonar)
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
    return 1; // the ECHO has begun
}


SA_FUNC uint16_t sonar_ping(SonarState* sonar)
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


SA_INLINE sonar_ping_cm(SonarState* state)
{
    return sonar_ping(state) / US_ROUNDTRIP_CM;
}


SA_INLINE sonar_ping_in(SonarState* state)
{
    return sonar_ping(state) / US_ROUNDTRIP_IN;
}
#endif // SANGSTER_SONAR_H
