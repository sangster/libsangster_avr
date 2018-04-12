#ifndef SANGSTER_API_H
#define SANGSTER_API_H
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
#ifndef SA_FUNC
#define SA_FUNC  extern
#endif//SA_FUNC

#ifndef SA_INLINE
#define SA_INLINE  extern inline
/*
 * TODO: Is always_inline useful?
 * #define SA_INLINE  __attribute__((always_inline)) static inline
 */
#endif//SA_INLINE

#endif//SANGSTER_API_H
