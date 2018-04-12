#ifndef SANGSTER_PCD8544_TEXT_H
#define SANGSTER_PCD8544_TEXT_H
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
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "sangster/pcd8544/draw.h"


/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define PCD_FONT_H     8 ///< The height of every character in every font
#define PCD_FONT_SPACE 0 ///< Additional space, in px, between string characters

/// @return The width of a string with `n` characters, in pixels
#define pcd_text_width(font, n) \
    (n * ((font).width + PCD_FONT_SPACE) - PCD_FONT_SPACE)


/*******************************************************************************
 * Types
 ******************************************************************************/
typedef struct pcd_font PcdFont;
struct pcd_font
{
    const PcdBank* chars; ///< The bitmap representing the font characters
    const size_t   width; ///< The width of every character, in px
    const char     first; ///< The first codepoint in this font
    const size_t   count; ///< Number of characters in this font
};


/*******************************************************************************
 * Function Declarations
 ******************************************************************************/
/**
 * Draws a string on the screen with the supplied font
 */
SA_FUNC void pcd_print(PcdDraw*, const PcdFont*, PcdIdx col, PcdIdx bank,
                       const char*, PcdColor);
/**
 * Draws a single character on the screen with the supplied font
 */
SA_FUNC void pcd_char(PcdDraw*, const PcdFont*, PcdIdx col, PcdIdx bank,
                      char, PcdColor);


/*******************************************************************************
 * Function Definitions
 ******************************************************************************/
SA_FUNC void pcd_print(PcdDraw* draw, const PcdFont* font, const PcdIdx x,
                       const PcdIdx y, const char* str, const PcdColor en)
{
    for (size_t i = 0; str[i]; ++i) {
        pcd_char(draw, font, x + (i * (font->width + PCD_FONT_SPACE)), y,
                 str[i], en);
    }
}


SA_FUNC void pcd_char(PcdDraw* draw, const PcdFont* font, const PcdIdx x,
                      const PcdIdx y, const char ch, const PcdColor en)
{
    if (ch < font->first || ch >= (char) (font->first + font->count)) {
        return; // Character is not supported by the given font
    }
    const uint8_t* glyph_start = font->chars + (ch - font->first) * font->width;

    for (size_t col = 0; col < font->width; ++col) {
        const PcdBank glyph = pgm_read_byte(glyph_start + col);

        for (size_t row = 0; row < PCD_FONT_H; ++row) {
            if (glyph & _BV(row)) {
                pcd_xy(draw, x + col, y + row, en);
            } else {
                pcd_xy(draw, x + col, y + row, !en);
            }
        }
    }
}
#endif//SANGSTER_PCD8544_TEXT_H
