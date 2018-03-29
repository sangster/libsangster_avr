#ifndef SANGSTER_LED_BUILTIN_H
#define SANGSTER_LED_BUILTIN_H
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
#include <avr/io.h>
#include "sangster/api.h"
#include "sangster/pinout.h"

/** The MPU pin controlling the Arduino builtin LED */
static const Pinout led_pin = {&PINB, PINB5};


/** Enable the Arduino builtin LED */
SA_INLINE void led_builtin_enable()
{
    pinout_make_output(led_pin);
}


/** Toggle the Arduino builtin LED */
SA_INLINE void led_builtin_toggle()
{
    pinout_set(led_pin);
}

#endif // SANGSTER_LED_BUILTIN_H
