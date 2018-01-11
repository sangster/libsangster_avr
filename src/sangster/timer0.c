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
#include "timer0.h"

volatile uint16_t _timer0_overflow_count = 0;
volatile uint16_t _timer0_millis = 0;
static uint8_t _timer0_fract = 0;

void timer0_interrupt_callback()
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



void timer0_start()
{
    TCCR0B |= _BV(CS01) | _BV(CS00); // timer0 prescaler: 1/64 (8 us / clk)
    TIMSK0 |= _BV(TOIE0);            // timer0 overflow interrupt

    /* TODO: I think these lines are unrelated to the timer! */
    /* // set a2d prescaler so we are inside the desired 50-200 KHz range. */
    /* ADCSRA |= _BV(ADPS2) | _BV(ADPS1) | _BV(ADPS0); */
    /* ADCSRA |= _BV(ADEN); // enable a2d conversions */
    timer0_reset();
}

void timer0_reset()
{
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        _timer0_overflow_count = 0;
        _timer0_millis = 0;
        _timer0_fract = 0;
        TCNT0 = 0;
    }
}


uint16_t timer0_ms()
{
    uint32_t m;

    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        m = _timer0_millis;
    }

    return m;
}


uint16_t timer0_us()
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
