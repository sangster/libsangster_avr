#ifndef SANGSTER_PCD8544_CORE_H
#define SANGSTER_PCD8544_CORE_H
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
#include <avr/io.h>
#include "sangster/api.h"
#include "sangster/pinout.h"


/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define PCD_COLS   84 ///< X-resolution of the LCD screen
#define PCD_ROWS   48 ///< Y-resolution of the LCD screen
#define PCD_BANKW   8 ///< The width of each DDRAM "bank", in bits
#define PCD_BANKS  (PCD_ROWS / PCD_BANKW) ///< Number of banks per col

/// The bank of the given row (0..PCD_BANKS)
#define pcd_y_to_bank(y)  ((y) / PCD_BANKW)

/// The bit offset (within a bank) of the given row (0..PCD_BANKW)
#define pcd_y_to_bit(y)   ((y) % PCD_BANKW)

#define PCD_MAX_OP_VOLTAGE 0x7F // 2^7 - 1

#define PCD_T_VHRL    100 ///< min. ms (VDD to RES LOW)

#define PCD_T_WL_RES  100 ///< min. ns (RES LOW pulse width)
#define PCD_T_CY      250 ///< min. ns (clock cycle SCLK)
#define PCD_T_WH1     100 ///< min. ns (SCLK pulse width HIGH)
#define PCD_T_WL1     100 ///< min. ns (SCLK pulse width LOW)
#define PCD_T_SU2      60 ///< min. ns (SCE set-up time)
#define PCD_T_H2      100 ///< min. ns (SCE hold tie)
#define PCD_T_WH2     100 ///< min. ns (SCE min. HIGH time)
#define PCD_T_H5      100 ///< min. ns (SCE start hold time)
#define PCD_T_SU3     100 ///< min. ns (D/C set-up time)
#define PCD_T_H3      100 ///< min. ns (D/C hold time)
#define PCD_T_SU4     100 ///< min. ns (SDIN set-up time)
#define PCD_T_H4      100 ///< min. ns (SDIN hold time)


/*******************************************************************************
 * Types
 ******************************************************************************/
#define PcdIdx    int8_t  ///< An index
#define PcdBank   uint8_t ///< A bitmap "bank" of 8 pixels in a column


enum pcd_command
{
    // Either Address set
    PCD_FUNCTION_SET     = _BV(5),

    // Basic instruction set (H = 0)
    PCD_DISPLAY_CONTROL  = _BV(3),
    PCD_SET_DDRAM_Y_ADDR = _BV(6),
    PCD_SET_DDRAM_X_ADDR = _BV(7),

    // Extended instruction set (H = 1)
    PCD_SET_TEMP_COEFF   = _BV(2),
    PCD_SET_BIAS         = _BV(4),
    PCD_SET_OP_VOLTAGE   = _BV(7),
};
typedef enum pcd_command PcdCommand;


enum pcd_function_set
{
    PCD_BASIC_INST    = 0,      // "H = 0" in the datasheet
    PCD_EXTENDED_INST = _BV(0), // "H = 1" in the datasheet

    PCD_HORIZ_ADDR    = 0,
    PCD_VERT_ADDR     = _BV(1),

    PCD_POWER_ACTIVE  = 0,
    PCD_POWER_DOWN    = _BV(2),
};
typedef enum pcd_function_set PcdFunctionSet;


enum pcd_display_mode
{
    PCD_DISPLAY_BLANK = 0,               // Not sure what this does
    PCD_SEGMENTS_ON   =          _BV(0), // Turn on every pixel
    PCD_NORMAL_MODE   = _BV(2),          // Not sure. Turns the screen on?
    PCD_INVERSE_VIDEO = _BV(2) | _BV(0), // Reverses each pixel
};
typedef enum pcd_display_mode PcdDisplayMode;


enum pcd_temperature_coeff
{
    PCD_TEMP_COEFF_0 = 0,
    PCD_TEMP_COEFF_1 =          _BV(0),
    PCD_TEMP_COEFF_2 = _BV(1),
    PCD_TEMP_COEFF_3 = _BV(1) | _BV(0),
};
typedef enum pcd_temperature_coeff PcdTemperatureCoeff;


enum pcd_bias
{
    PCD_BIAS_100 = 0,
    PCD_BIAS_80  =                   _BV(0),
    PCD_BIAS_65  =          _BV(1),
    PCD_BIAS_48  =          _BV(1) | _BV(0),
    PCD_BIAS_34  = _BV(2),
    PCD_BIAS_24  = _BV(2) |          _BV(0),
    PCD_BIAS_16  = _BV(2) | _BV(1),
    PCD_BIAS_8   = _BV(2) | _BV(1) | _BV(0),
};
typedef enum pcd_bias PcdBias;


enum pcd_mode
{
    PCD_COMMAND = 0,
    PCD_DATA    = 1,
};
typedef enum pcd_mode PcdMode;


typedef struct pcd Pcd;
struct pcd
{
    const Pinout pin_sce_; // active low
    const Pinout pin_res_; // active low
    const Pinout pin_dc;
    const Pinout pin_sdin;
    const Pinout pin_sclk;
    const Pinout pin_led;

    PcdFunctionSet inst_set;
    PcdFunctionSet addr_dir;
    PcdFunctionSet power;

    PcdDisplayMode display_mode;
    PcdTemperatureCoeff temp_coeff;
    PcdBias bias;
    uint8_t op_voltage;

    PcdIdx bank; ///< Y address
    PcdIdx col;  ///< X address
};


/*******************************************************************************
 * Function Declarations
 ******************************************************************************/
/**
 * Must be called within T_VHRL milliseconds of providing power to the LCD.
 */
SA_FUNC void pcd_setup(Pcd*);

/// Resets the screen to its start-up state
SA_FUNC void pcd_res_pulse(Pcd*);

/// Send a single byte to the screen
SA_FUNC void pcd_send_byte(const Pcd*, uint8_t);
SA_FUNC void pcd_instruction_set(Pcd*, PcdFunctionSet);
SA_FUNC void pcd_address_direction(Pcd*, PcdFunctionSet);
SA_FUNC void pcd_power_state(Pcd*, PcdFunctionSet);
SA_FUNC void pcd_update_function_set(const Pcd*);
SA_FUNC void pcd_data(Pcd*, PcdBank);
SA_INLINE void pcd_cmd(const Pcd*, uint8_t);
SA_FUNC void pcd_send(const Pcd*, PcdMode, uint8_t);
SA_FUNC void pcd_display(Pcd*, PcdDisplayMode);
SA_INLINE void pcd_move(Pcd*, PcdIdx col, PcdIdx bank);
SA_FUNC void pcd_col(Pcd*, PcdIdx);
SA_FUNC void pcd_bank(Pcd*, PcdIdx);
SA_FUNC void pcd_temperature_coeff(Pcd*, PcdTemperatureCoeff);
SA_FUNC void pcd_op_voltage(Pcd*, uint8_t);
SA_FUNC void pcd_bias(Pcd*, const PcdBias);

/**
 * Clears the screen.
 *
 * @note The state of the DDRAM is undefined upon start-up, so unless you
 *   redraw every pixel immediately, call this after pcd_setup().
 */
SA_FUNC void pcd_clr_all(const Pcd*);


/*******************************************************************************
 * Function Definitions
 ******************************************************************************/
SA_FUNC void pcd_setup(Pcd* pcd)
{
    pinout_make_output(pcd->pin_sce_);
    pinout_make_output(pcd->pin_res_);
    pinout_make_output(pcd->pin_dc);
    pinout_make_output(pcd->pin_sdin);
    pinout_make_output(pcd->pin_sclk);
    pinout_make_output(pcd->pin_led);

    pcd_res_pulse(pcd);
    pcd_temperature_coeff(pcd, PCD_TEMP_COEFF_0);
    pcd_bias(pcd, PCD_BIAS_48); // per "GG0804A1FSN6G" LCD datasheet
    pcd_power_state(pcd, PCD_POWER_ACTIVE);
    pcd_display(pcd, PCD_NORMAL_MODE);
}


SA_FUNC void pcd_res_pulse(Pcd* pcd)
{
    pinout_clr(pcd->pin_res_);
    pinout_set(pcd->pin_res_);

    // Post-reset settings according to datasheet
    pcd->inst_set     = PCD_BASIC_INST;
    pcd->addr_dir     = PCD_HORIZ_ADDR;
    pcd->power        = PCD_POWER_DOWN;
    pcd->display_mode = PCD_DISPLAY_BLANK;
    pcd->temp_coeff   = PCD_TEMP_COEFF_0;
    pcd->bias         = PCD_BIAS_100;
}


SA_FUNC void pcd_instruction_set(Pcd* pcd, const PcdFunctionSet inst)
{
    if (pcd->inst_set != inst) {
        pcd->inst_set = inst;
        pcd_update_function_set(pcd);
    }
}


SA_FUNC void pcd_address_direction(Pcd* pcd, const PcdFunctionSet dir)
{
    if (pcd->addr_dir != dir) {
        pcd->addr_dir = dir;
        pcd_update_function_set(pcd);
    }
}


SA_FUNC void pcd_power_state(Pcd* pcd, const PcdFunctionSet pwr)
{
    if (pcd->power != pwr) {
        pcd->power = pwr;
        pcd_update_function_set(pcd);
    }
}


SA_FUNC void pcd_update_function_set(const Pcd* pcd)
{
    pcd_cmd(pcd,
            PCD_FUNCTION_SET | (pcd->inst_set | pcd->addr_dir | pcd->power));

    if (pcd->power == PCD_POWER_ACTIVE) {
        pinout_set(pcd->pin_led);
    } else {
        pinout_clr(pcd->pin_led);
    }
}


SA_FUNC void pcd_data(Pcd* pcd, const PcdBank byte)
{
    pcd_send(pcd, PCD_DATA, byte);

    if (pcd->addr_dir == PCD_HORIZ_ADDR) {
        pcd->col = (pcd->col + 1) % PCD_COLS;
        if (pcd->col == 0) {
            pcd->bank = (pcd->bank + 1) % PCD_BANKS;
        }
    } else {
        pcd->bank = (pcd->bank + 1) % PCD_BANKS;
        if (pcd->bank == 0) {
            pcd->col = (pcd->col + 1) % PCD_COLS;
        }
    }
}


SA_INLINE void pcd_cmd(const Pcd* pcd, const uint8_t byte)
{
    pcd_send(pcd, PCD_COMMAND, byte);
}


SA_FUNC void pcd_send(const Pcd* pcd, const PcdMode mode, const uint8_t byte)
{
    if (mode == PCD_DATA) {
        pinout_set(pcd->pin_dc);
    } else {
        pinout_clr(pcd->pin_dc);
    }

    pinout_clr(pcd->pin_sce_);
    pcd_send_byte(pcd, byte);
    pinout_set(pcd->pin_sce_);
}


SA_FUNC void pcd_send_byte(const Pcd* pcd, const uint8_t byte)
{
    for (uint8_t mask = 0x80; mask; mask >>= 1) {
        if (byte & mask) {
            pinout_set(pcd->pin_sdin);
        } else {
            pinout_clr(pcd->pin_sdin);
        }
        pinout_set(pcd->pin_sclk);
        pinout_clr(pcd->pin_sclk);
    }
}


SA_FUNC void pcd_display(Pcd* pcd, const PcdDisplayMode mode)
{
    if (pcd->display_mode != mode) {
        pcd->display_mode = mode;
        pcd_instruction_set(pcd, PCD_BASIC_INST);
        pcd_cmd(pcd, PCD_DISPLAY_CONTROL | mode);
    }
}


SA_INLINE void pcd_move(Pcd* pcd, PcdIdx col, PcdIdx bank)
{
    pcd_col(pcd, col);
    pcd_bank(pcd, bank);
}


SA_FUNC void pcd_col(Pcd* pcd, PcdIdx col)
{
    col %= PCD_COLS;
    if (pcd->col != col) {
        pcd->col = col;
        pcd_instruction_set(pcd, PCD_BASIC_INST);
        pcd_cmd(pcd, PCD_SET_DDRAM_X_ADDR | pcd->col);
    }
}


SA_FUNC void pcd_bank(Pcd* pcd, PcdIdx bank)
{
    bank %= PCD_BANKS;
    if (pcd->bank != bank) {
        pcd->bank = bank;
        pcd_instruction_set(pcd, PCD_BASIC_INST);
        pcd_cmd(pcd, PCD_SET_DDRAM_Y_ADDR | pcd->bank);
    }
}


SA_FUNC void pcd_temperature_coeff(Pcd* pcd, const PcdTemperatureCoeff coeff)
{
    if (pcd->temp_coeff != coeff) {
        pcd->temp_coeff = coeff;
        pcd_instruction_set(pcd, PCD_EXTENDED_INST);
        pcd_cmd(pcd, PCD_SET_TEMP_COEFF | coeff);
    }
}


SA_FUNC void pcd_bias(Pcd* pcd, const PcdBias bias)
{
    if (pcd->bias != bias) {
        pcd->bias = bias;
        pcd_instruction_set(pcd, PCD_EXTENDED_INST);
        pcd_cmd(pcd, PCD_SET_BIAS | bias);
    }
}


SA_FUNC void pcd_op_voltage(Pcd* pcd, uint8_t voltage)
{
    if (voltage > PCD_MAX_OP_VOLTAGE) {
        voltage = PCD_MAX_OP_VOLTAGE;
    }
    if (pcd->op_voltage != voltage) {
        pcd->op_voltage = voltage;

        pcd_instruction_set(pcd, PCD_EXTENDED_INST);
        pcd_cmd(pcd, PCD_SET_OP_VOLTAGE | voltage);
    }
}


SA_FUNC void pcd_clr_all(const Pcd* pcd)
{
    for (uint16_t i = 0; i < PCD_COLS * PCD_BANKS; ++i) {
        pcd_send(pcd, PCD_DATA, 0x00);
    }
}
#endif//SANGSTER_PCD8544_CORE_H
