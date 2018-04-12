#ifndef SANGSTER_PINOUT_H
#define SANGSTER_PINOUT_H
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
 * Provides helpers to make using microcontroller pins with the library more
 * readable.
 *
 * This doesn't handle setting a pin as `IN` or `OUT`, or enabling pull-up
 * resistors, so the caller must.
 */
#include <stddef.h>
#include <avr/io.h>
#include <avr/sfr_defs.h>


/*******************************************************************************
 * Types
 ******************************************************************************/
/*
 * A register/pin-offset pair to allow microcontroller pins to be passed to
 * library functions in a more readable fashion.
 *
 * All the members are `const`, so these structs should be optimized out
 * entirely, resulting in the same machine code as when hard-coding the port
 * references.
 */
typedef struct pinout Pinout;
struct pinout
{
    /*
     * TODO: Can these be const? Can you reassign a struct with const members?
     */

    volatile uint8_t* const reg; ///< an I/O register. ex: `PORTB`, `PINC`
    const uint8_t pin;           ///< The offset of the pin. ex: `PORTB4`
};


/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define pinout_offset(x, first) (&first + ((x.reg - &PINB) / 3) * 3)
#define pinout_null(x) ((x).reg == NULL)

/** Sets the pin HIGH. */
#define pinout_set(x) (*(x).reg |= _BV((x).pin))
/** Sets the pin LOW. */
#define pinout_clr(x) (*(x).reg &= ~_BV((x).pin))

/** Sets the pin HIGH if LOW or LOW if HIGH. */
#define pinout_toggle(x) (*(pinout_offset(x, PINB)) |= _BV((x).pin))

/** Wrapper for `bit_is_set`. */
#define pinout_is_set(x) bit_is_set(*(pinout_offset(x, PINB)), ((x).pin))
/** Wrapper for `bit_is_clear`. */
#define pinout_is_clr(x) bit_is_clear(*(pinout_offset(x, PINB)), ((x).pin))

/** Wrapper for `loop_until_bit_is_set`. */
#define pinout_until_set(x) loop_until_bit_is_set(*(x).reg, ((x).pin))
/** Wrapper for `loop_until_bit_is_clear`. */
#define pinout_until_clr(x) loop_until_bit_is_clear(*(x).reg, ((x).pin))

/*
 * The registers are in groups of three, PINx:DDRx:PORTx, (where x IN (B,C,D))
 * starting with PINB. This / 3 * 3 code uses integer division to map either
 * PINX or PORTx to their DDRx register.
 */
/** Configure this pin as an output, via DDRx. */
#define pinout_make_output(x) (*(pinout_offset(x, DDRB)) |= _BV((x).pin))
/** Configure this pin as an input, via DDRx. */
#define pinout_make_input(x) (*(pinout_offset(x, DDRB)) &= ~_BV((x).pin))

/** Configure this pin as an input and enable the pull-up resistor . */
#define pinout_make_pullup_input(x) \
    do { pinout_make_input(x); pinout_set(x); } while(0)

/**
 * @defgroup atmega328p_pins ATMega328/p Pin Definitions
 */
/// @{
#define PIN_DEF_ATMEGA328P_RX    {&PORTD, PORTD0}
#define PIN_DEF_ATMEGA328P_TX    {&PORTD, PORTD1}

#define PIN_DEF_ATMEGA328P_INT0  {&PORTD, PORTD2}
#define PIN_DEF_ATMEGA328P_INT1  {&PORTD, PORTD3}

#define PIN_DEF_ATMEGA328P_SS    {&PORTB, PORTB2}
#define PIN_DEF_ATMEGA328P_MOSI  {&PORTB, PORTB3}
#define PIN_DEF_ATMEGA328P_MISO  {&PORTB, PORTB4}
#define PIN_DEF_ATMEGA328P_SCK   {&PORTB, PORTB5}

#define PIN_DEF_ATMEGA328P_SDA   {&PORTC, PORTC4}
#define PIN_DEF_ATMEGA328P_SCL   {&PORTC, PORTC5}
/// @}

/**
 * @defgroup arduino_pins Arduino Pin Definitions
 */
/// @{
#define PIN_DEF_ARDUINO_0   {&PORTD, PORTD0}
#define PIN_DEF_ARDUINO_1   {&PORTD, PORTD1}
#define PIN_DEF_ARDUINO_2   {&PORTD, PORTD2}
#define PIN_DEF_ARDUINO_3   {&PORTD, PORTD3}
#define PIN_DEF_ARDUINO_4   {&PORTD, PORTD4}
#define PIN_DEF_ARDUINO_5   {&PORTD, PORTD5}
#define PIN_DEF_ARDUINO_6   {&PORTD, PORTD6}
#define PIN_DEF_ARDUINO_7   {&PORTD, PORTD7}
#define PIN_DEF_ARDUINO_8   {&PORTB, PORTB0}
#define PIN_DEF_ARDUINO_9   {&PORTB, PORTB1}
#define PIN_DEF_ARDUINO_10  {&PORTB, PORTB2}
#define PIN_DEF_ARDUINO_11  {&PORTB, PORTB3}
#define PIN_DEF_ARDUINO_12  {&PORTB, PORTB4}
#define PIN_DEF_ARDUINO_13  {&PORTB, PORTB5}
#define PIN_DEF_ARDUINO_14  {&PORTC, PORTC0}
#define PIN_DEF_ARDUINO_15  {&PORTC, PORTC1}
#define PIN_DEF_ARDUINO_16  {&PORTC, PORTC2}
#define PIN_DEF_ARDUINO_17  {&PORTC, PORTC3}
#define PIN_DEF_ARDUINO_18  {&PORTC, PORTC4}
#define PIN_DEF_ARDUINO_19  {&PORTC, PORTC5}

#define PIN_DEF_ARDUINO_A0  PIN_DEF_ARDUINO_14
#define PIN_DEF_ARDUINO_A1  PIN_DEF_ARDUINO_15
#define PIN_DEF_ARDUINO_A2  PIN_DEF_ARDUINO_16
#define PIN_DEF_ARDUINO_A3  PIN_DEF_ARDUINO_17
#define PIN_DEF_ARDUINO_A4  PIN_DEF_ARDUINO_18
#define PIN_DEF_ARDUINO_A5  PIN_DEF_ARDUINO_19
/// @}

#endif // SANGSTER_PINOUT_H
