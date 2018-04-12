#ifndef SANGSTER_PCD8544_DRAW_H
#define SANGSTER_PCD8544_DRAW_H
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
#include "sangster/pcd8544/core.h"

#define PCD_WHITE  false ///< The PcdColor represented by 0
#define PCD_BLACK  true  ///< The PcdColor represented by 1

/// Swaps x and y if y < x
#define PCD_IDX_ORDER(x,y)  do { PcdIdx t_ ## x ; \
                                 if (y<x) { t_ ## x = x; x = y; y = t_ ## x; } \
                            } while(0)


/*******************************************************************************
 * Types
 ******************************************************************************/
/// This LCD supports 1-bit color
typedef bool PcdColor;

/// Callback function when a PCD "bank" is updated
typedef void    (*PcdUpdate)(void* payload, PcdIdx col, PcdIdx bank, PcdBank);

/// Callback function when the current value of a PCD "bank" is needed
typedef PcdBank (*PcdGet)(const void*, PcdIdx col, PcdIdx bank);

typedef struct pcd_draw PcdDraw;
struct pcd_draw
{
    Pcd* pcd;
    PcdBank buffer[PCD_BANKS * PCD_COLS]; ///< Bitmap currently draw on the LCD

    void* context; ///< Payload passed to PcdUpdate and PcdGet

    /// Callback function when a PCD "bank" is updated
    PcdUpdate update_func;

    /// Callback function when the current value of a PCD "bank" is needed
    PcdGet get_func;
};


/*******************************************************************************
 * Function Declarations
 ******************************************************************************/
/// Configure the given PcdDraw for use
SA_FUNC void pcd_draw_init(PcdDraw*, Pcd*);

/// Fill in the entire screen
SA_INLINE void pcd_fill(PcdDraw*, PcdColor);

/// Draw a single pixel
SA_FUNC void pcd_xy(PcdDraw*, PcdIdx x, PcdIdx y, PcdColor);

/// Draw a straight line
SA_FUNC void pcd_line(PcdDraw*, PcdIdx x1, PcdIdx y1, PcdIdx x2, PcdIdx y2,
                     PcdColor);

/**
 * Fill in a rectangle
 *
 * @param x1  The col of the top-left corner
 * @param y1  The row of the top-left corner
 * @param x2  The col of the bottom-right corner
 * @param y2  The row of the bottom-right corner
 */
SA_FUNC void pcd_rect(PcdDraw*, PcdIdx x1, PcdIdx y1, PcdIdx x2, PcdIdx y2,
                     PcdColor);

/**
 * Draw the border of a circle
 *
 * @param x  The col of the center of the circle
 * @param y  The row of the center of the circle
 */
SA_FUNC void pcd_ring(PcdDraw*, PcdIdx x, PcdIdx y, PcdIdx radius, PcdColor);

/**
 * Fill in a circle
 *
 * @param x  The col of the center of the circle
 * @param y  The row of the center of the circle
 */
SA_FUNC void pcd_circ(PcdDraw*, PcdIdx x, PcdIdx y, PcdIdx radius, PcdColor);

/// The default implementation of PcdDraw.get_func
SA_FUNC PcdBank pcd_default_get_func(const void*, PcdIdx col, PcdIdx bank);

/**
 * The default implementation of PcdDraw.update_func
 *
 * Updates the buffer and sends the new bank to the LCD screen
 */
SA_FUNC void pcd_default_update_func(void*, PcdIdx x, PcdIdx y, PcdBank);

/**
 * @return The array index for the given bank
 *
 * @note The index depends on Pcd.addr_dir, to match the LCD's DDRAM. See Fig.5
 *   and Fig.6 in the PCD8544 datasheet for diagrams.
 */
SA_INLINE uint16_t pcd_bank_idx(const PcdDraw*, PcdIdx col, PcdIdx bank);

/**
 * Update the given DDRAM bank with a new value.
 */
SA_INLINE void pcd_update_bank(PcdDraw*, PcdIdx col, PcdIdx bank, PcdBank);


/*******************************************************************************
 * Function Definitions
 ******************************************************************************/
SA_FUNC void pcd_draw_init(PcdDraw* draw, Pcd* pcd)
{
    draw->pcd = pcd;
    memset(draw->buffer, 0x00, PCD_BANKS * PCD_COLS);
    draw->context = (void*) draw;
    draw->update_func = pcd_default_update_func;
    draw->get_func = pcd_default_get_func;
}


SA_INLINE void pcd_fill(PcdDraw* draw, const PcdColor en)
{
    pcd_rect(draw, 0, 0, PCD_COLS - 1, PCD_ROWS - 1, en);
}


SA_FUNC void pcd_xy(PcdDraw* draw, PcdIdx x, PcdIdx y, const PcdColor en)
{
    x %= PCD_COLS;
    y %= PCD_ROWS;

    const PcdIdx bank = pcd_y_to_bank(y);
    const PcdIdx bit  = pcd_y_to_bit(y);
    const PcdBank current_bank = draw->get_func(draw->context, x, bank);

    PcdBank new_bank = current_bank;
    if (en) {
        new_bank |= 1 << bit;
    } else {
        new_bank &= ~(1 << bit);
    }

    draw->update_func(draw->context, x, pcd_y_to_bank(y), new_bank);
    pcd_update_bank(draw, x, pcd_y_to_bank(y), new_bank);
}


SA_INLINE void pcd_update_bank(PcdDraw* draw, PcdIdx col, PcdIdx bank, PcdBank bankval)
{
    draw->update_func(draw->context, col, bank, bankval);
}


SA_FUNC void pcd_line(PcdDraw* draw, PcdIdx x1, const PcdIdx y1, PcdIdx x2,
              const PcdIdx y2, const PcdColor en)
{
    PCD_IDX_ORDER(x1, x2);

    const PcdIdx dx = x2 - x1;
    if (dx == 0) { // vertical line
        for (PcdIdx y = y1; y <= y2; ++y) {
            pcd_xy(draw, x1, y, en);
        }
        return;
    }
    const PcdIdx dy = y2 - y1;

    for (PcdIdx x = x1; x <= x2; ++x) {
        const PcdIdx y = y1 + dy * (x - x1) / dx;
        pcd_xy(draw, x, y, en);
    }
}


SA_FUNC void pcd_rect(PcdDraw* draw, PcdIdx x1, PcdIdx y1, PcdIdx x2, PcdIdx y2,
              const PcdColor en)
{
    PCD_IDX_ORDER(x1, x2);
    PCD_IDX_ORDER(y1, y2);
    for (PcdIdx x = x1; x <= x2; ++x) {
        for (PcdIdx y = y1; y <= y2; ++y) {
            pcd_xy(draw, x, y, en);
        }
    }
}


SA_FUNC void pcd_ring(PcdDraw* draw, PcdIdx cx, const PcdIdx cy, const PcdIdx radius,
              const PcdColor en)
{
    PcdIdx x = radius - 1;
    PcdIdx y = 0;
    PcdIdx dx = 1;
    PcdIdx dy = 1;
    PcdIdx err = dx - (radius << 1);

    while (x >= y) {
        pcd_xy(draw, cx + x, cy + y, en);
        pcd_xy(draw, cx + y, cy + x, en);
        pcd_xy(draw, cx - y, cy + x, en);
        pcd_xy(draw, cx - x, cy + y, en);
        pcd_xy(draw, cx - x, cy - y, en);
        pcd_xy(draw, cx - y, cy - x, en);
        pcd_xy(draw, cx + y, cy - x, en);
        pcd_xy(draw, cx + x, cy - y, en);

        if (err <= 0) {
            ++y;
            err += dy;
            dy += 2;
        }
        if (err > 0) {
            --x;
            dx += 2;
            err += dx - (radius << 1);
        }
    }
}


SA_FUNC void pcd_circ(PcdDraw* draw, PcdIdx cx, const PcdIdx cy, const PcdIdx radius,
              const PcdColor en)
{
    PcdIdx x = radius - 1;
    PcdIdx y = 0;
    PcdIdx dx = 1;
    PcdIdx dy = 1;
    PcdIdx err = dx - (radius << 1);

    while (x >= y) {
        pcd_line(draw, cx - x, cy - y, cx + x, cy - y, en);
        pcd_line(draw, cx - y, cy - x, cx + y, cy - x, en);

        pcd_line(draw, cx - y, cy + x, cx + y, cy + x, en);
        pcd_line(draw, cx - x, cy + y, cx + x, cy + y, en);

        if (err <= 0) {
            ++y;
            err += dy;
            dy += 2;
        }
        if (err > 0) {
            --x;
            dx += 2;
            err += dx - (radius << 1);
        }
    }
}


SA_INLINE PcdBank pcd_default_get_func(const void* context, const PcdIdx col,
                             const PcdIdx bank)
{
    const PcdDraw* const draw = context;
    return draw->buffer[pcd_bank_idx(draw, col, bank)];
}


SA_FUNC void pcd_default_update_func(void* payload, const PcdIdx col,
                             const PcdIdx bank, const PcdBank bank_val)
{
    PcdDraw* const draw = (PcdDraw*) payload;
    const uint16_t idx = pcd_bank_idx(draw, col, bank);

    draw->buffer[idx] = bank_val;
    pcd_move(draw->pcd, col, bank);
    pcd_data(draw->pcd, bank_val);
}


SA_INLINE uint16_t pcd_bank_idx(const PcdDraw* draw, const PcdIdx col,
                                const PcdIdx bank)
{
    if (draw->pcd->addr_dir == PCD_HORIZ_ADDR) {
        return bank * PCD_COLS + col;
    } else {
        return col * PCD_BANKS + bank;
    }
}
#endif//SANGSTER_PCD8544_DRAW_H
