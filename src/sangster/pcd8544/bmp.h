#ifndef SANGSTER_PCD8544_BMP_H
#define SANGSTER_PCD8544_BMP_H
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
#include <stdint.h>
#include <avr/pgmspace.h>
#include "sangster/pcd8544/draw.h"


/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define PCD_BMP_BANKS(img) (((img).height / 8) + !!((img).height % 8))
#define PCD_BMP_CX_OFFSET(img) ((PCD_COLS - (img).width) / 2)
#define PCD_BMP_CY_OFFSET(img) ((PCD_BANKS - PCD_BMP_BANKS(img)) / 2)


/*******************************************************************************
 * Types
 ******************************************************************************/
typedef struct pcd_bitmap PcdBitmap;
struct pcd_bitmap
{
    const PcdBank* data;
    size_t         width;  // in pixels
    size_t         height; // in pixels
};


/*******************************************************************************
 * Function Declarations
 ******************************************************************************/
SA_FUNC void pcd_bmp_draw(PcdDraw*, const PcdBitmap*);
SA_INLINE void pcd_bmp_draw_center(PcdDraw*, const PcdBitmap*);


/*******************************************************************************
 * Function Definitions
 ******************************************************************************/
SA_FUNC void pcd_bmp_draw(PcdDraw* draw, const PcdBitmap* img)
{
    const size_t banks = PCD_BMP_BANKS(*img),
                 start_col = draw->pcd->col,
                 start_bank = draw->pcd->bank;

    for (size_t y = 0; y < banks; ++y) {
        for (size_t x = 0, w = img->width; x < w; ++x) {
            pcd_update_bank(draw, start_col + x, start_bank +y,
                            pgm_read_byte(img->data + (y * w + x)));
        }
    }
}


SA_INLINE void pcd_bmp_draw_center(PcdDraw* draw, const PcdBitmap* img)
{
    pcd_move(draw->pcd, PCD_BMP_CX_OFFSET(*img), PCD_BMP_CY_OFFSET(*img));
    pcd_bmp_draw(draw, img);
}
#endif//SANGSTER_PCD8544_BMP_H
