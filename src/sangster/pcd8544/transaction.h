#ifndef SANGSTER_PCD8544_TRANSACTION_H
#define SANGSTER_PCD8544_TRANSACTION_H
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
#include <string.h>
#include "sangster/pcd8544/draw.h"

/*******************************************************************************
 * Types
 ******************************************************************************/
typedef struct pcd_trans PcdTrans;
struct pcd_trans
{
    PcdDraw* draw;
    PcdBank newbuff[PCD_BANKS * PCD_COLS];

    void* old_context;
    PcdUpdate old_update_func;
    PcdGet old_get_func;
};


/*******************************************************************************
 * Function Declarations
 ******************************************************************************/
/**
 * Starts a new transaction
 *
 * Futher uses of `draw`'s functions will not result in the LCD screen being
 * updated. The results of these calls can be sent to the screen in one pass
 * with pcd_trans_commit(). The changes can be discarded with pcd_trans_abort().
 */
SA_FUNC void pcd_trans_start(PcdTrans* tr, PcdDraw* draw);

/**
 * Updates the attached PCD8544 LCD screen and returns the PcdDraw object to
 * its original functionality.
 */
SA_FUNC void pcd_trans_commit(PcdTrans* tr);

/**
 * Abandons any changes made since the start of the transaction and returns the
 * PcdDraw object to its original functionality.
 */
SA_INLINE void pcd_trans_abort(PcdTrans* tr);

/// Update callback that stores the changes without updating the device
SA_FUNC void    pcd_trans_update_func(void*, PcdIdx x, PcdIdx y, PcdBank);

/// Returns the currently-uncommited bank
SA_FUNC PcdBank pcd_trans_get(const void*, PcdIdx col, PcdIdx bank);


/*******************************************************************************
 * Function Definitions
 ******************************************************************************/
SA_FUNC void pcd_trans_start(PcdTrans* tr, PcdDraw* draw)
{
    tr->old_context = draw->context;
    draw->context = tr;

    tr->old_update_func = draw->update_func;
    draw->update_func = pcd_trans_update_func;

    tr->old_get_func = draw->get_func;
    draw->get_func = pcd_trans_get;

    tr->draw = draw;
    memcpy(tr->newbuff, draw->buffer, PCD_BANKS * PCD_COLS);
}


SA_FUNC void pcd_trans_commit(PcdTrans* tr)
{
    for (PcdIdx bank = 0; bank < PCD_BANKS; ++bank) {
        for (PcdIdx col = 0; col < PCD_COLS; ++col) {
            const uint16_t idx = pcd_bank_idx(tr->draw, col, bank);
            const PcdBank val = tr->newbuff[idx];

            if (val != tr->draw->buffer[idx]) {
                tr->draw->buffer[idx] = val;
                pcd_move(tr->draw->pcd, col, bank);
                pcd_data(tr->draw->pcd, val);
            }
        }
    }

    pcd_trans_abort(tr); // just to restore the PcdDraw attibutes
}


SA_FUNC void pcd_trans_update_func(void* payload, const PcdIdx col,
                                   const PcdIdx bank, const PcdBank bank_val)
{
    PcdTrans* const tr = (PcdTrans*) payload;
    tr->newbuff[pcd_bank_idx(tr->draw, col, bank)] = bank_val;
}


SA_FUNC PcdBank pcd_trans_get(const void* context, const PcdIdx col,
                              const PcdIdx bank)
{
    const PcdTrans* const trans = context;
    return trans->newbuff[pcd_bank_idx(trans->draw, col, bank)];
}


SA_INLINE void pcd_trans_abort(PcdTrans* tr)
{
    tr->draw->context = tr->old_context;
    tr->draw->update_func = tr->old_update_func;
    tr->draw->get_func = tr->old_get_func;
}
#endif//SANGSTER_PCD8544_TRANSACTION_H
