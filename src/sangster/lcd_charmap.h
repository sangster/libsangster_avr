#ifndef LCD_CHARMAP_H
#define LCD_CHARMAP_H
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
 * Extra, non-ASCII characters supported by the LCD
 */
enum lcd_char
{
    LCD_PLUS_MINUS = 0x10,
    LCD_IDENTICAL_TO,              ///< ≡
    LCD_UNKNOWN_1,
    LCD_UNKNOWN_2,
    LCD_CURVE_TOP_RIGHT,
    LCD_CURVE_BOTTOM_RIGHT,
    LCD_CURVE_TOP_LEFT,
    LCD_CURVE_BOTTOM_LEFT,
    LCD_CURVE,
    LCD_CURVE_BACK,
    LCD_ALMOST_EQUAL_TO,           ///< ≈
    LCD_INTEGRAL,
    LCD_EQUALS_LOW,
    LCD_SIN_WAVE,
    LCD_SUPERSCRIPT_TWO,           ///< ²
    LCD_SUPERSCRIPT_THREE,         ///< ³

    // ASCII Chars here

    LCD_TRIANGLE_UP       = 0x7F,
    LCD_CAPITAL_C_CEDILLA = 0x80,  ///< Ç
    LCD_SMALL_U_DIAERESIS,         ///< ü
    LCD_SMALL_E_ACUTE,             ///< é
    LCD_SMALL_A_CIRCUMFLEX,        ///< â
    LCD_SMALL_A_DIARESIS,          ///< ä
    LCD_SMALL_A_GRAVE,             ///< à
    LCD_SMALL_A_RING,              ///< å
    LCD_SMALL_C_CEDILLA,           ///< ç
    LCD_SMALL_E_CIRCUMFLEX,        ///< ê
    LCD_SMALL_E_DIARESIS,          ///< ë
    LCD_SMALL_E_GRAVE,             ///< è
    LCD_SMALL_I_DIARESIS,          ///< ë
    LCD_SMALL_I_CIRCUMFLEX,        ///< î
    LCD_SMALL_I_GRAVE,             ///< ì
    LCD_CAPITAL_A_DIARESIS,        ///< Ä
    LCD_CAPITAL_A_RING,            ///< Å
    LCD_CAPITAL_E_ACUTE   = 0x90,  ///< É
    LCD_SMALL_AE,                  ///< æ
    LCD_CAPITAL_AE,                ///< Æ
    LCD_SMALL_O_CIRCUMFLEX,        ///< ô
    LCD_SMALL_O_DIARESIS,          ///< ö
    LCD_SMALL_O_GRAVE,             ///< ò
    LCD_SMALL_U_CIRCUMFLEX,        ///< û
    LCD_SMALL_U_GRAVE,             ///< ù
    LCD_SMALL_Y_DIARESIS,          ///< ÿ
    LCD_CAPITAL_O_DIARESIS,        ///< Ö
    LCD_CAPITAL_U_DIARESIS,        ///< Ü
    LCD_SMALL_N_TILDE,             ///< ñ
    LCD_CAPITAL_N_TILDE,           ///< Ñ
    LCD_SMALL_A_UNDERLINE,
    LCD_SMALL_O_UNDERLINE,
    LCD_INVERTED_QUESTION_MARK,    ///< ¿
    LCD_SMALL_A_ACUTE     = 0xA0,  ///< á
    LCD_SMALL_I_ACUTE,             ///< í
    LCD_SMALL_O_ACUTE,             ///< ó
    LCD_CENT_SIGN,                 ///< ¢
    LCD_POUND_SIGN,                ///< £
    LCD_YEN_SIGN,                  ///< ¥
    LCD_PESETA_SIGN,               ///< ₧
    LCD_FINITE_INTEGRAL,           ///< ⨍
    LCD_INVERTED_EXCLAMATION_MARK, ///< ¡
    LCD_CAPITAL_A_TILDE,           ///< Ã
    LCD_SMALL_A_TILDE,             ///< ã
    LCD_CAPITAL_O_TILDE,           ///< Õ
    LCD_SMALL_O_TILDE,
    LCD_CAPITAL_O_STROKE,          ///< Ø
    LCD_SMALL_O_STROKE,            ///< ø
    LCD_UNKNOWN_3         = 0xB0,
    LCD_UNKNOWN_4,
    LCD_DEGREE,                    ///< °
    LCD_UNKNOWN_5,
    LCD_UNKNOWN_6,
    LCD_ONE_HALF,                  ///< ½
    LCD_ONE_QUARTER,               ///< ¼
    LCD_MULTIPLICATION_SIGN,       ///< ×
    LCD_DIVISON_SIGN,              ///< ÷
    LCD_LESS_THAN_OR_EQUAL_TO,     ///< ≤
    LCD_GREATER_THAN_OR_EQUAL_TO,  ///< ≥
    LCD_MUCH_LESS_THAN,            ///< «
    LCD_MUCH_GREATER_THAN,         ///< »
    LCD_NOT_EQUAL_TO,              ///< ≠
    LCD_SQUARE_ROOT,               ///< √
    LCD_OVERLINE,                  ///< ‾
    LCD_UNKNOWN_7         = 0xC0,
    LCD_UNKNOWN_8,
    LCD_INFINITY,                  ///< ∞
    LCD_TRIANGLE_UPPER_LEFT,       ///< ◸
    LCD_ARROW_DOWN_LEFT,           ///< ↲
    LCD_ARROW_UP,                  ///< ↑
    LCD_ARROW_DOWN,                ///< ↓
    LCD_ARROW_RIGHT,               ///< →
    LCD_ARROW_LEFT,                ///< ←
    LCD_BOX_TOP_LEFT,              ///< ┌
    LCD_BOX_TOP_RIGHT,             ///< ┐
    LCD_BOX_BOTTOM_LEFT,           ///< └
    LCD_BOX_BOTTOM_RIGHT,          ///< ┘
    LCD_MIDDLE_DOT,                ///< ·
    LCD_REGISTERED_SIGN,           ///< ®
    LCD_COPYRIGHT_SIGN,            ///< ©
    LCD_TRADEMARK_SIGN    = 0xD0,  ///< ™
    LCD_DAGGER,                    ///< †
    LCD_SECTION_SIGN,              ///< §
    LCD_PILCROW_SIGN,              ///< ¶
    LCD_UNKNOWN_9,
    LCD_TRIANGLE_RIGHT,
    LCD_CAPITAL_THETA,             ///< ϴ
    LCD_CAPITAL_LAMDA,             ///< Λ
    LCD_CAPITAL_XI,                ///< Ξ
    LCD_CAPITAL_PI,                ///< Π
    LCD_CAPITAL_SIGMA,             ///< Σ
    LCD_CAPITAL_UPSILON,           ///< Υ
    LCD_CAPITAL_PHI,               ///< Φ
    LCD_CAPITAL_PSI,               ///< Ψ
    LCD_CAPITAL_OMEGA,             ///< Ω
    LCD_SMALL_ALPHA,               ///< α
    LCD_SMALL_BETA        = 0xE0,  ///< β
    LCD_SMALL_GAMMA,               ///< γ
    LCD_SMALL_DELTA,               ///< δ
    LCD_SMALL_EPSILON,             ///< ε
    LCD_SMALL_ZETA,                ///< ζ
    LCD_SMALL_ETA,                 ///< η
    LCD_SMALL_THETA,               ///< θ
    LCD_SMALL_IOTA,                ///< ι
    LCD_SMALL_KAPPA,               ///< κ
    LCD_SMALL_LAMDA,               ///< λ
    LCD_SMALL_MU,                  ///< μ
    LCD_SMALL_NU,                  ///< ν
    LCD_SMALL_XI,                  ///< ξ
    LCD_SMALL_PI,                  ///< π
    LCD_SMALL_RHO,                 ///< ρ
    LCD_SMALL_SIGMA,               ///< σ
    LCD_SMALL_TAU         = 0xF0,  ///< τ
    LCD_SMALL_UPSILON,             ///< υ
    LCD_SMALL_CHI,                 ///< χ
    LCD_SMALL_PSI,                 ///< ψ
    LCD_SMALL_OMGEA,               ///< ω
    LCD_BLACK_TRIANGLE_DOWN,       ///< ▼
    LCD_BLACK_TRIANGLE_RIGHT,      ///< ▶
    LCD_BLACK_TRIANGLE_LEFT,       ///< ◀
    LCD_R_BOLD,
    LCD_ARROW_LEFT_FROM_BAR,       ///< ↤
    LCD_F_BOLD,
    LCD_ARROW_RIGHT_FROM_BAR,      ///< ↦
    LCD_SQUARE,                    ///< ⬜
    LCD_BOLD_HYPHEN,
    LCD_S_REVERSE,
    LCD_P_REVERSE,
};
typedef enum lcd_char LcdChar;

#endif//LCD_CHARMAP_H
