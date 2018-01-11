#ifndef SANGSTER_TIMER0_H
#define SANGSTER_TIMER0_H
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
 *
 * Uses the 8-bit TIMER0 to keep a `uint32_t` counter of elapsed real time,
 * since the last call of timer0_reset().
 */
#include <sangster/timer.h>
#include <avr/io.h>
#include <util/atomic.h>


/** How long, in microseconds, it takes for TIMER0 to overflow.  */
#define US_PER_TIMER0_OVF ((64 * 256) / CYCLES_PER_US)

/** the whole number of milliseconds per timer0 overflow */
#define TIMER0_MILLIS_INC (US_PER_TIMER0_OVF / 1000)

/*
 * The fractional number of milliseconds per TIMER0 overflow. We shift right
 * by three to fit these numbers into a byte (for the clock speed we care
 * about, 16 MHz, this doesn't lose precision).
 */
#define TIMER0_FRACT_INC ((US_PER_TIMER0_OVF % 1000) >> 3)

/** The portion of a byte that represents a full unit. */
#define TIMER0_FRACT_MAX (1000 >> 3)

void timer0_interrupt_callback();

/**
 * Configures TIMER0 with a prescaler of 1/64 (8us per increment) and starts
 * the timer.
 *
 * @note This timer relies on interrupts to increase its counter, so you
 *   **must** enable interrupts with `sei()` before calling this function.
 */
void timer0_start();

/** Restart the timer from 0 */
void timer0_reset();

/** @return The time since `TIMER0` last retarted, in milliseconds. */
uint16_t timer0_ms();

/** @return The time since `TIMER0` last retarted, in microseconds. */
uint16_t timer0_us();


#endif // SANGSTER_TIMER0_H
