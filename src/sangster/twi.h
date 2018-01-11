#ifndef SANGSTER_TWI_H
#define SANGSTER_TWI_H
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
#include <stdlib.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/twi.h>
#include <sangster/pinout.h>

/**
 * TWI can run at two speeds:
 *  - normal: 100 kHz
 *  - fast:   400 kHz
 *
 * However, the RTC 1307 RTC only runs at normal speed
 */
#ifndef TWI_SCL_FREQ
#define TWI_SCL_FREQ 100e3 // 100 kHz
#endif//TWI_SCL_FREQ

#ifndef TWI_BUFF_LEN
#define TWI_BUFF_LEN 32
#endif//TWI_BUFF_LEN

enum twi_state
{
    TWI_READY = 0,

    TWI_MASTER_RX = 1,
    TWI_MASTER_TX = 2,

    TWI_SLAVE_RX = 3,
    TWI_SLAVE_TX = 4
};
typedef enum twi_state TwiState;

enum twi_write_res
{
    TWI_WRITE_GOOD = 0,
    TWI_WRITE_NOT_SLAVE = 1,
    TWI_WRITE_BUFF_FULL = 2
};
typedef enum twi_write_res TwiWriteRes;

enum twi_bus_write_res
{
    TWI_BUS_WRITE_GOOD = 0,
    TWI_BUS_WRITE_TOO_LONG = 1,
    TWI_BUS_ADDR_NACK = 2,
    TWI_BUS_DATA_NACK = 3,
    TWI_BUS_OTHER_ERR = 4
};
typedef enum twi_bus_write_res TwiBusWriteRes;

enum twi_ack_opt
{
    TWI_ACK_DONT_SEND = 0,
    TWI_ACK_SEND = 1
};
typedef enum twi_ack_opt TwiAckOpt;


struct twi
{
    // PC4/5 have extra hardware for this purpose. see p.266
    Pinout pin_sda; // PORTC4 pin
    Pinout pin_scl; // PORTC5 pin

    volatile TwiState state;
    volatile uint8_t send_stop;
    volatile uint8_t in_repeated_start;
    volatile uint8_t error;

    uint8_t is_transmitting;
    volatile uint8_t slave_rw;

    uint8_t rx_buff[TWI_BUFF_LEN];
    volatile uint8_t rx_buff_idx;
    volatile uint8_t rx_buff_len;

    uint8_t tx_addr;
    uint8_t tx_buff[TWI_BUFF_LEN];
    volatile uint8_t tx_buff_idx;
    volatile uint8_t tx_buff_len;

    uint8_t master_buff[TWI_BUFF_LEN];
    volatile uint8_t master_buff_idx;
    volatile uint8_t master_buff_len;

    void (*on_slave_recv)(uint8_t*, uint8_t);
    void (*on_slave_send)();
};
typedef struct twi Twi;


void twi_handle_vect();
void twi_init(Twi* twi);
void twi_disable();
void twi_begin_tx(uint8_t addr);
TwiBusWriteRes twi_end_tx(uint8_t send_stop);
TwiBusWriteRes twi_bus_write(uint8_t wait, uint8_t send_stop);
void twi_reply(TwiAckOpt send_ack);
void twi_stop();
void twi_release_bus();
uint8_t twi_bus_request(uint8_t address, size_t size, uint32_t iaddress,
                        size_t isize, uint8_t send_stop);
uint8_t twi_bus_read(uint8_t address, uint8_t *data, size_t size,
                     uint8_t send_stop);

/**
 * May only be called in:
 *   - slave tx callback
 *   - after twi_begin_tx()
 */
size_t twi_write(uint8_t data);
int16_t twi_read();


#endif//SANGSTER_TWI_H
