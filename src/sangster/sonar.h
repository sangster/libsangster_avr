#ifndef SANGSTER_SONAR_H
#define SANGSTER_SONAR_H
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

/**
 * @file
 */
#include <stdint.h>
#include <avr/io.h>
#include <util/delay.h>
#include <sangster/timer0.h>
#include "pinout.h"

/**
 * In addition to the speed of sound in air, the trigger function itself takes
 * some time to execute. This time must be included when calculating the
 * duration of the trigger/echo sequence.
 */
#define PING_OVERHEAD 5 // microseconds

/** time it takes to travel 1 cm, in us (from sensor datasheet) */
#define US_ROUNDTRIP_CM 58u

/** time it takes to travel 1", in us (from sensor datasheet) */
#define US_ROUNDTRIP_IN 148u

/** max sensor start-up time, in us */
#define MAX_SENSOR_DELAY 18000u

/** The number of clock cycles which ellapse per millisecond. */
#define TICKS_PER_MS(x, sc) ((x) * ((F_CPU) / ((sc) * 1000u)))

/** The number of clock cycles which ellapse per microsecond. */
#define TICKS_PER_US(x, sc) ((x) * ((F_CPU) / ((sc) * 1000000u)))


typedef struct sonar_state SonarState;
struct sonar_state
{
    const Pinout trigger; ///< [out] The pin to trigger the ping
    const Pinout capture; ///< [out] Configures the echo interrupt
    const Pinout interrupt; ///< [out] Clears the echo interrupt
    const Pinout overflow; ///< [out] Clears the interrupt overflow

    uint16_t max_distance_cm;
    uint16_t start_at;
    uint16_t timeout;
};


/**
 * @return How long it took for the echo to return, in microseconds. `0` if no
 *   echo was received (likely the maximum distance was exceeded).
 */
uint16_t sonar_ping(SonarState*);


/** @return The distance measured by the sensor, in centimeters. */
__attribute__((always_inline))
inline uint16_t sonar_ping_cm(SonarState* state)
{
    return sonar_ping(state) / US_ROUNDTRIP_CM;
}


/** @return The distance measured by the sensor, in inches. */
__attribute__((always_inline))
inline uint16_t sonar_ping_in(SonarState* state)
{
    return sonar_ping(state) / US_ROUNDTRIP_IN;
}

/**
 * Triggers the device to send out the ping and waits for its echo signal.
 *
 * @return `1` if the signal was successfully sent and the `echo` has begun.
 * `0` if the ping timed-out.
 */
uint8_t ping_trigger(SonarState*);

#endif // SANGSTER_SONAR_H
