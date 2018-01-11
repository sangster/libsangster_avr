#ifndef SANGSTER_LCD_H
#define SANGSTER_LCD_H
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
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include "lcd_charmap.h"
#include "pinout.h"

/** The number of bytes in the LCD's DRAM that separates each row */
#define LCD_ROW_OFFSET 0x40

/** The number of possible rows */
#define LCD_ROWS ((uint8_t) 2)

/** The number of columns in each row */
#define LCD_COLS ((uint8_t) 16)

#define BOOL_SET(x, y, z) do{ if (x) { y |= (z); } else {  y &= ~(z); } }while(0)


/** LCD Commands and their instruction codes */
enum lcd_command {
    LCD_CLEAR_DISPLAY   = 0x01, ///< Blank the screen
    LCD_RETURN_HOME     = 0x02, ///< Return the cursor to the first position
    LCD_ENTRY_MODE_SET  = 0x04, ///< Setup how new characters appear
    LCD_DISPLAY_CONTROL = 0x08, ///< Setup the LCD screen
    LCD_FUNCTION_SET    = 0x20, ///< Setup # of lines, font size, etc.
    LCD_SET_DDRAM_ADDR  = 0x80  ///< Choose the cursor location
};

enum { // Display Entry Mode
    LCD_ENTRY_RIGHT = 0x00, ///< Text is printed right-to-left
    LCD_ENTRY_LEFT  = 0x02  ///< Text is printed left-to-right
};
enum {
    LCD_ENTRY_SHIFT_DECREMENT = 0x00, ///< Shift to the left
    LCD_ENTRY_SHIFT_INCREMENT = 0x01  ///< Shift to the right
};
enum lcd_display_control {
    LCD_BLINK   = 0x01, ///< Turn blinking on/off
    LCD_CURSOR  = 0x02, ///< Turn the cursor on/off
    LCD_DISPLAY = 0x04, ///< Turn the screen on/off
};
enum { // LCD flags: function set
    LCD_MODE_4BIT = 0x00, ///< Use 4 pins to transfer data
    LCD_MODE_8BIT = 0x10  ///< Use 8 pins to transfer data
};
enum {
    LCD_LINES_1 = 0x00, ///< Use 1 line of text
    LCD_LINES_2 = 0x08  ///< Use 2 lines of text
};
enum {
    LCD_DOTS_5x8  = 0x00, ///< Use a 5x8 font
    LCD_DOTS_5x10 = 0x04  ///< Use a 5x10 font
};
enum { // Write Modes
    LCD_MODE_CMD = 0x00, ///< Send a command to the LCD
    LCD_MODE_DAT = 0x01  ///< Send character data to the LCD
};

typedef struct lcd Lcd;


/**
 * Allows the caller to provide custom functionality for sending data to the
 * LCD, one nibble at a time
 */
typedef void (*LcdWrite4)(const Lcd *lcd, const uint8_t nibble);


struct lcd
{
    const Pinout rs; ///< [out]
    const Pinout en; ///< [out]
    const Pinout pins[4]; ///< [out]

    uint8_t display_function;
    uint8_t display_mode;
    uint8_t display_control;
    uint8_t num_rows;
    char (*back_buffer)[LCD_ROWS][LCD_COLS];

    LcdWrite4 write4;
};


void lcd_init(Lcd*, uint8_t rows, LcdWrite4);
void lcd_begin(Lcd*, const uint8_t rows, const uint8_t dot_size);

/** Erase all characters from the LCD screen */
void lcd_clear(const Lcd*);

/**
 * Add a character to the LCD back-buffer. The LCD will only be updated if the
 * buffer actually changes.
 */
void lcd_buffer_send_at(const Lcd* lcd, uint8_t row, uint8_t col, char ch);

/**
 * Add a substring to the LCD back-buffer. The LCD will only be updated if the
 * buffer actually changes.
 */
void lcd_buffer_update_at(const Lcd*, uint8_t row, uint8_t col, char const*);

/**
 * Add a substring from PMG space to the LCD back-buffer. The LCD will only be
 * updated if the buffer actually changes.
 */
void lcd_buffer_update_at_P(const Lcd*, uint8_t row, uint8_t col, char const*);


/** Return the cursor to its initial position. */
void lcd_home(const Lcd*);

/** Turn the display on or off. */
void lcd_display(Lcd*, bool enabled);

/** Turn the cursor on or off. */
void lcd_cursor(Lcd*, bool enabled);

/** Start or stop blinking. */
void lcd_blink(Lcd*, bool enabled);


/**
 * @param lcd The interfacing device
 */
void lcd_send(const Lcd*, const uint8_t value, const uint8_t mode);


/**
 * Moves the cursor on the LCD; where the next character will be placed. If
 * @ref LCD_BLINK_ON is set, the cursor will blink at this location.
 *
 * @param lcd The interfacing device
 * @param row,col The location to move the cursor to
 */
void lcd_move_cursor(const Lcd*, uint8_t row, const uint8_t col);


/**
 * Prints the given character on the LCD, at it's cursor location.
 *
 * @param lcd The interfacing device
 * @param ch The character to add to the display
 */
__attribute__((always_inline))
static inline void lcd_write(const Lcd* lcd, const uint8_t ch)
{
    lcd_send(lcd, ch, LCD_MODE_DAT);
}


/**
 * Prints `n` characters of the the given text on the LCD, starting at it's
 * cursor location.
 *
 * @param lcd The interfacing device
 * @param str The new text to display
 * @param n   The number of characters to print
 */
size_t lcd_writen(const Lcd*, const char*, size_t n);


/**
 * Prints `n` characters of the the given text from PGMSPACE on the LCD,
 * starting at it's cursor location.
 *
 * @param lcd The interfacing device
 * @param str The new text to display, from PGMSPACE
 * @param n   The number of characters to print
 */
size_t lcd_writen_P(const Lcd*, const char*, size_t n);


/**
 * Prints the given text on the LCD, starting at it's cursor location.
 *
 * @param lcd The interfacing device
 * @param str The new text to display
 */
__attribute__((always_inline))
static inline void lcd_print(const Lcd* lcd, const char* str)
{
    while(*str) {
        lcd_write(lcd, *str++);
    }
}


/**
 * Prints the given text from PGMSPACE on the LCD, starting at it's cursor
 * location.
 *
 * @param lcd The interfacing device
 * @param str The new text to display
 */
__attribute__((always_inline))
static inline void lcd_print_P(const Lcd* lcd, const char* str)
{
    char byte;
    while ((byte = pgm_read_byte(str++))) {
        lcd_write(lcd, byte);
    }
}


/**
 * Clear the LCD and print the given text.
 *
 * @param lcd The interfacing device
 * @param str The new text to display
 */
__attribute__((always_inline))
static inline void lcd_reprint(const Lcd* lcd, const char* str)
{
    lcd_clear(lcd);
    lcd_print(lcd, str);
}


/**
 * Clear the LCD and print the given text from PGMSPACE.
 *
 * @param lcd The interfacing device
 * @param str The new text to display
 */
__attribute__((always_inline))
static inline void lcd_reprint_P(const Lcd* lcd, const char* str)
{
    lcd_clear(lcd);
    lcd_print_P(lcd, str);
}


/**
 * Replace an entire line in the back-buffer. The LCD will only be updated if
 * the buffer has changed.
 */
__attribute__((always_inline))
static inline void lcd_buffer_update(const Lcd* lcd, uint8_t row, const char* str)
{
    lcd_buffer_update_at(lcd, row, 0, str);
}



/**
 * Replace an entire line in the back-buffer with a string from PGM space. The
 * LCD will only be updated if the buffer has changed.
 */
__attribute__((always_inline))
static inline void lcd_buffer_update_P(const Lcd* lcd, uint8_t row, PGM_P str)
{
    lcd_buffer_update_at_P(lcd, row, 0, str);
}

#endif//SANGSTER LCD_H
