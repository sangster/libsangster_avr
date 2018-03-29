#ifndef SANGSTER_LCD_H
#define SANGSTER_LCD_H
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
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include "sangster/api.h"
#include "sangster/lcd_charmap.h"
#include "sangster/pinout.h"

/** The number of bytes in the LCD's DRAM that separates each row */
#define LCD_ROW_OFFSET 0x40

/** The number of possible rows */
#define LCD_ROWS ((uint8_t) 2)

/** The number of columns in each row */
#define LCD_COLS ((uint8_t) 16)

#define BOOL_SET(x, y, z) do{ if (x) { y |= (z); } else {  y &= ~(z); } }while(0)


/** LCD Commands and their instruction codes */
enum lcd_command
{
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


/** The default LcdWrite4 implementation. */
SA_FUNC void default_lcd_write4(const Lcd* lcd, const uint8_t nibble)
{
    for (uint8_t i = 0; i < 4; ++i) {
        if ((nibble >> i) & 0x01) {
            pinout_set(lcd->pins[i]);
        } else {
            pinout_clr(lcd->pins[i]);
        }
    }
}


/** Flips the EN I/O port a couple times to end a write */
SA_FUNC void lcd_pulse(const Lcd* lcd)
{
    pinout_clr(lcd->en);
    _delay_us(5);

    pinout_set(lcd->en);
    _delay_us(5);

    pinout_clr(lcd->en);
    _delay_us(100);
}


/** Send 1 nibble of data to the LCD */
SA_FUNC void lcd_write4(const Lcd* lcd, const uint8_t nibble)
{
    if (lcd->write4 != NULL) {
        lcd->write4(lcd, nibble);
    } else {
        default_lcd_write4(lcd, nibble);
    }
    lcd_pulse(lcd);
}


/**
 * @param lcd The interfacing device
 */
SA_FUNC void lcd_send(const Lcd* lcd, const uint8_t value, const uint8_t mode)
{
    if (mode) {
        pinout_set(lcd->rs);
    } else {
        pinout_clr(lcd->rs);
    }

    if (lcd->display_function & LCD_MODE_8BIT) {
        // TODO: 8-bit
    } else {
        lcd_write4(lcd, value >> 4);
        lcd_write4(lcd, value);
    }
}


SA_INLINE void lcd_command(const Lcd* lcd, const uint8_t value)
{
    lcd_send(lcd, value, LCD_MODE_CMD);
}


/** Turn the display on or off. */
SA_FUNC void lcd_display(Lcd* lcd, const bool enabled)
{
    BOOL_SET(enabled, lcd->display_control, LCD_DISPLAY);
    lcd_command(lcd, LCD_DISPLAY_CONTROL | lcd->display_control);
}


/** Turn the cursor on or off. */
SA_FUNC void lcd_cursor(Lcd* lcd, const bool enabled)
{
    BOOL_SET(enabled, lcd->display_control, LCD_CURSOR);
    lcd_command(lcd, LCD_DISPLAY_CONTROL | lcd->display_control);
}


/** Start or stop blinking. */
SA_FUNC void lcd_blink(Lcd* lcd, const bool enabled)
{
    BOOL_SET(enabled, lcd->display_control, LCD_BLINK);
    lcd_command(lcd, LCD_DISPLAY_CONTROL | lcd->display_control);
}


/** Erase all characters from the LCD screen */
SA_FUNC void lcd_clear(const Lcd* lcd)
{
    lcd_command(lcd, LCD_CLEAR_DISPLAY);
    _delay_us(3000);
}


SA_FUNC void lcd_begin(Lcd* lcd, const uint8_t rows, const uint8_t dot_size)
{
    if (rows > 1) {
        lcd->display_function |= LCD_LINES_2;
    }

    lcd->num_rows = rows;

    // some devices can have a taller font? may be useless here
    if (dot_size != LCD_DOTS_5x8 && rows == 1) {
        lcd->display_function |= LCD_DOTS_5x10;
    }

    // 40+ ms warm-up time
    _delay_ms(50);

    if (!(lcd->display_function & LCD_MODE_8BIT)) {
        // 4 bit mode, see figure 24 from HD44780.PDF
        lcd_write4(lcd, 0x03); // 1st try
        _delay_us(4500);
        lcd_write4(lcd, 0x03); // 2nd try
        _delay_us(4500);
        lcd_write4(lcd, 0x03); // last try
        _delay_us(150);

        lcd_write4(lcd, 0x02);
    } else {
        // TODO
    }

    // set # lines, font size, etc.
    lcd_command(lcd, LCD_FUNCTION_SET | lcd->display_function);

    lcd->display_control = LCD_DISPLAY & ~(LCD_CURSOR | LCD_BLINK);
    lcd_display(lcd, true);
    lcd_clear(lcd);

    lcd->display_mode = LCD_ENTRY_LEFT | LCD_ENTRY_SHIFT_DECREMENT;
    lcd_command(lcd, LCD_ENTRY_MODE_SET | lcd->display_mode);
}


/*
 * @param write4 an optional write method
 */
SA_FUNC void lcd_init(Lcd* lcd, const uint8_t rows, LcdWrite4 write4)
{
    lcd->display_function = LCD_MODE_4BIT | LCD_LINES_1 | LCD_DOTS_5x8;
    lcd->write4 = write4;
    lcd_begin(lcd, rows, LCD_DOTS_5x8);
}


/**
 * Moves the cursor on the LCD; where the next character will be placed. If
 * @ref LCD_BLINK_ON is set, the cursor will blink at this location.
 *
 * @param lcd The interfacing device
 * @param row,col The location to move the cursor to
 */
SA_FUNC void lcd_move_cursor(const Lcd* lcd, uint8_t row, const uint8_t col)
{
    if (row >= LCD_ROWS) {
        row = LCD_ROWS - 1;
    }
    if (row >= lcd->num_rows) {
        row = lcd->num_rows - 1;
    }

    lcd_command(lcd, LCD_SET_DDRAM_ADDR | (col + LCD_ROW_OFFSET * row));
}


/**
 * Prints the given character on the LCD, at it's cursor location.
 *
 * @param lcd The interfacing device
 * @param ch The character to add to the display
 */
SA_INLINE void lcd_write(const Lcd* lcd, const uint8_t ch)
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
SA_FUNC size_t lcd_writen(const Lcd* lcd, const char* str, size_t n)
{
    for (size_t i = 0; i < n; ++i) {
        if (!str[i]) {
            return i;
        }
        lcd_write(lcd, str[i]);
    }
    return n;
}


/**
 * Prints `n` characters of the the given text from PGMSPACE on the LCD,
 * starting at it's cursor location.
 *
 * @param lcd The interfacing device
 * @param str The new text to display, from PGMSPACE
 * @param n   The number of characters to print
 */
SA_FUNC size_t lcd_writen_P(const Lcd* lcd, const char* str, size_t n)
{
    for (size_t i = 0; i < n; ++i) {
        uint8_t byte = pgm_read_byte(&str[i]);
        if (!byte) {
            return i;
        }
        lcd_write(lcd, byte);
    }
    return n;
}


/**
 * Prints the given text on the LCD, starting at it's cursor location.
 *
 * @param lcd The interfacing device
 * @param str The new text to display
 */
SA_INLINE void lcd_print(const Lcd* lcd, const char* str)
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
SA_INLINE void lcd_print_P(const Lcd* lcd, const char* str)
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
SA_INLINE void lcd_reprint(const Lcd* lcd, const char* str)
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
SA_INLINE void lcd_reprint_P(const Lcd* lcd, const char* str)
{
    lcd_clear(lcd);
    lcd_print_P(lcd, str);
}


SA_FUNC void lcd_buffer_reprint(const Lcd* lcd, const uint8_t row,
                                const uint8_t col)
{
    lcd_move_cursor(lcd, row, col);
    size_t size = lcd_writen(lcd, &(*lcd->back_buffer)[row][col], LCD_COLS - col);
    while(size++ < (size_t) LCD_COLS) {
        lcd_write(lcd, ' ');
    }
}


/**
 * Add a character to the LCD back-buffer. The LCD will only be updated if the
 * buffer actually changes.
 */
SA_FUNC void lcd_buffer_send_at(const Lcd* lcd, const uint8_t row,
                               const uint8_t col, const char ch)
{
    if (ch == (*lcd->back_buffer)[row][col]) {
        return;
    }
    (*lcd->back_buffer)[row][col] = ch;
    lcd_buffer_reprint(lcd, row, col);
}


/**
 * Add a substring to the LCD back-buffer. The LCD will only be updated if the
 * buffer actually changes.
 */
SA_FUNC void lcd_buffer_update_at(const Lcd* lcd, const uint8_t row,
                                 const uint8_t col, const char* str)
{
    char* substr = &(*lcd->back_buffer)[row][col];
    if (strncmp(substr, str, LCD_COLS - col) == 0) {
        return;
    }
    strncpy(substr, str, LCD_COLS - col);
    lcd_buffer_reprint(lcd, row, col);
}


/**
 * Add a substring from PMG space to the LCD back-buffer. The LCD will only be
 * updated if the buffer actually changes.
 */
SA_FUNC void lcd_buffer_update_at_P(const Lcd* lcd, uint8_t row, uint8_t col,
                                   const char* str)
{
    char* substr = &(*lcd->back_buffer)[row][col];
    if (strncmp_P(substr, str, LCD_COLS - col) == 0) {
        return;
    }
    strncpy_P(substr, str, LCD_COLS - col);
    lcd_buffer_reprint(lcd, row, col);
}


/** Return the cursor to its initial position. */
SA_FUNC void lcd_home(const Lcd* lcd)
{
    lcd_command(lcd, LCD_RETURN_HOME);
    _delay_us(3000);
}


/**
 * Replace an entire line in the back-buffer. The LCD will only be updated if
 * the buffer has changed.
 */
SA_INLINE void lcd_buffer_update(const Lcd* lcd, uint8_t row, const char* str)
{
    lcd_buffer_update_at(lcd, row, 0, str);
}


/**
 * Replace an entire line in the back-buffer with a string from PGM space. The
 * LCD will only be updated if the buffer has changed.
 */
SA_INLINE void lcd_buffer_update_P(const Lcd* lcd, uint8_t row, PGM_P str)
{
    lcd_buffer_update_at_P(lcd, row, 0, str);
}
#endif//SANGSTER LCD_H
