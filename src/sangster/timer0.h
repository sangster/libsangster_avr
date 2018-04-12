#ifndef SANGSTER_TIMER0_H
#define SANGSTER_TIMER0_H
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
 * Uses the 8-bit TIMER0 to keep a `uint32_t` counter of elapsed real time,
 * since the last call of timer0_reset().
 */
#include <avr/io.h>
#include <util/atomic.h>
#include "sangster/api.h"
#include "sangster/timer.h"


/*******************************************************************************
 * Definitions
 ******************************************************************************/
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


/*******************************************************************************
 * Global Data
 ******************************************************************************/
volatile uint16_t _timer0_overflow_count;
volatile uint16_t _timer0_millis;
uint8_t _timer0_fract;


/*******************************************************************************
 * Function Declarations
 ******************************************************************************/
SA_INLINE void timer0_interrupt_callback();

/// Restart the timer from 0
SA_FUNC void timer0_reset();

/**
 * Configures TIMER0 with a prescaler of 1/64 (8us per increment) and starts
 * the timer.
 *
 * @note This timer relies on interrupts to increase its counter, so you
 *   **must** enable interrupts with `sei()` before calling this function.
 */
SA_FUNC void timer0_start();

/// @return The time since `TIMER0` last retarted, in milliseconds
SA_FUNC uint16_t timer0_ms();

/// @return The time since `TIMER0` last retarted, in microseconds
SA_FUNC uint16_t timer0_us();


/*******************************************************************************
 * Function Definitions
 ******************************************************************************/
SA_INLINE void timer0_interrupt_callback()
{
    // copy these to local variables so they can be stored in registers
    // (volatile variables must be read from memory on every access)
    uint16_t m = _timer0_millis;
    uint16_t f = _timer0_fract;

    m += TIMER0_MILLIS_INC;
    f += TIMER0_FRACT_INC;
    if (f >= TIMER0_FRACT_MAX) {
        f -= TIMER0_FRACT_MAX;
        m += 1;
    }

    _timer0_fract = f;
    _timer0_millis = m;
    _timer0_overflow_count++;
}


SA_FUNC void timer0_reset()
{
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        _timer0_overflow_count = 0;
        _timer0_millis = 0;
        _timer0_fract = 0;
        TCNT0 = 0;
    }
}


SA_FUNC void timer0_start()
{
    TCCR0B |= _BV(CS01) | _BV(CS00); // timer0 prescaler: 1/64 (8 us / clk)
    TIMSK0 |= _BV(TOIE0);            // timer0 overflow interrupt

    /* TODO: I think these lines are unrelated to the timer! */
    /* // set a2d prescaler so we are inside the desired 50-200 KHz range. */
    /* ADCSRA |= _BV(ADPS2) | _BV(ADPS1) | _BV(ADPS0); */
    /* ADCSRA |= _BV(ADEN); // enable a2d conversions */
    timer0_reset();
}


/** @return The time since `TIMER0` last retarted, in milliseconds. */
SA_FUNC uint16_t timer0_ms()
{
    uint32_t m;

    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        m = _timer0_millis;
    }

    return m;
}


/** @return The time since `TIMER0` last retarted, in microseconds. */
SA_FUNC uint16_t timer0_us()
{
    uint16_t m;
    uint8_t t;

    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        m = _timer0_overflow_count;
        t = TCNT0;

        if (bit_is_set(TIFR0, TOV0) && t < 0xFF) {
            m++;
        }
    }

    return ((m << 8) + t) * (64 / CYCLES_PER_US);
}
#endif//SANGSTER_TIMER0_H
