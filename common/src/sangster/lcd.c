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
#include "lcd.h"

/** Flips the EN I/O port a couple times to end a write */
void lcd_pulse(const Lcd *);

/** Send 1 nibble of data to the LCD */
void lcd_write4(const Lcd*, const uint8_t value);

void lcd_set_offsets(const Lcd*, const uint8_t row0, const uint8_t row1,
                                 const uint8_t row2, const uint8_t row3);
inline void lcd_command(const Lcd*, const uint8_t value);

extern void lcd_write(const Lcd*, const uint8_t);
extern void lcd_print(const Lcd*, const char*);
extern void lcd_reprint(const Lcd*, const char*);
extern void lcd_print_P(const Lcd*, const char*);
extern void lcd_reprint_P(const Lcd*, const char*);


/** The default LcdWrite4 implementation. */
static void default_lcd_write4(const Lcd* lcd, const uint8_t nibble)
{
    for (uint8_t i = 0; i < 4; ++i) {
        if ((nibble >> i) & 0x01) {
            pinout_set(lcd->pins[i]);
        } else {
            pinout_clr(lcd->pins[i]);
        }
    }
}


/*
 * @param write4 an optional write method
 */
void lcd_init(Lcd* lcd, const uint8_t rows, LcdWrite4 write4)
{
    lcd->display_function = LCD_MODE_4BIT | LCD_LINES_1 | LCD_DOTS_5x8;
    lcd->write4 = write4;
    lcd_begin(lcd, rows, LCD_DOTS_5x8);
}


void lcd_begin(Lcd* lcd, uint8_t rows, uint8_t dot_size)
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


void lcd_clear(const Lcd* lcd)
{
    lcd_command(lcd, LCD_CLEAR_DISPLAY);
    _delay_us(3000);
}


void lcd_home(const Lcd* lcd)
{
    lcd_command(lcd, LCD_RETURN_HOME);
    _delay_us(3000);
}


void lcd_display(Lcd* lcd, bool enabled)
{
    BOOL_SET(enabled, lcd->display_control, LCD_DISPLAY);
    lcd_command(lcd, LCD_DISPLAY_CONTROL | lcd->display_control);
}


void lcd_cursor(Lcd* lcd, bool enabled)
{
    BOOL_SET(enabled, lcd->display_control, LCD_CURSOR);
    lcd_command(lcd, LCD_DISPLAY_CONTROL | lcd->display_control);
}


void lcd_blink(Lcd* lcd, bool enabled)
{
    BOOL_SET(enabled, lcd->display_control, LCD_BLINK);
    lcd_command(lcd, LCD_DISPLAY_CONTROL | lcd->display_control);
}


void lcd_send(const Lcd* lcd, uint8_t value, uint8_t mode)
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


__attribute__((always_inline))
inline void lcd_command(const Lcd* lcd, const uint8_t value)
{
    lcd_send(lcd, value, LCD_MODE_CMD);
}


void lcd_move_cursor(const Lcd* lcd, uint8_t row, const uint8_t col)
{
    if (row >= LCD_ROWS) {
        row = LCD_ROWS - 1;
    }
    if (row >= lcd->num_rows) {
        row = lcd->num_rows - 1;
    }

    lcd_command(lcd, LCD_SET_DDRAM_ADDR | (col + LCD_ROW_OFFSET * row));
}


void lcd_write4(const Lcd* lcd, const uint8_t nibble)
{
    if (lcd->write4 != NULL) {
        lcd->write4(lcd, nibble);
    } else {
        default_lcd_write4(lcd, nibble);
    }
    lcd_pulse(lcd);
}


void lcd_pulse(const Lcd* lcd)
{
    pinout_clr(lcd->en);
    _delay_us(5);

    pinout_set(lcd->en);
    _delay_us(5);

    pinout_clr(lcd->en);
    _delay_us(100);
}


size_t lcd_writen(const Lcd* lcd, const char* str, size_t n)
{
    for (size_t i = 0; i < n; ++i) {
        if (!str[i]) {
            return i;
        }
        lcd_write(lcd, str[i]);
    }
    return n;
}


size_t lcd_writen_P(const Lcd* lcd, const char* str, size_t n)
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


static void lcd_buffer_reprint(const Lcd* lcd, const uint8_t row,
                               const uint8_t col)
{
    lcd_move_cursor(lcd, row, col);
    size_t size = lcd_writen(lcd, &(*lcd->back_buffer)[row][col], LCD_COLS - col);
    while(size++ < (size_t) LCD_COLS) {
        lcd_write(lcd, ' ');
    }
}


void lcd_buffer_send_at(const Lcd* lcd, const uint8_t row, const uint8_t col,
                        const char ch)
{
    if (ch == (*lcd->back_buffer)[row][col]) {
        return;
    }
    (*lcd->back_buffer)[row][col] = ch;
    lcd_buffer_reprint(lcd, row, col);
}


void lcd_buffer_update_at(const Lcd* lcd, const uint8_t row, const uint8_t col,
                          const char* str)
{
    char* substr = &(*lcd->back_buffer)[row][col];
    if (strncmp(substr, str, LCD_COLS - col) == 0) {
        return;
    }
    strncpy(substr, str, LCD_COLS - col);
    lcd_buffer_reprint(lcd, row, col);
}


void lcd_buffer_update_at_P(const Lcd* lcd, uint8_t row, uint8_t col,
                            const char* str)
{
    char* substr = &(*lcd->back_buffer)[row][col];
    if (strncmp_P(substr, str, LCD_COLS - col) == 0) {
        return;
    }
    strncpy_P(substr, str, LCD_COLS - col);
    lcd_buffer_reprint(lcd, row, col);
}
