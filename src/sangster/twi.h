#ifndef SANGSTER_TWI_H
#define SANGSTER_TWI_H
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
#include <stdlib.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/twi.h>
#include "sangster/api.h"
#include "sangster/pinout.h"


/*******************************************************************************
 * Definitions
 ******************************************************************************/
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


/*******************************************************************************
 * Types
 ******************************************************************************/
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


typedef struct twi Twi;
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


/*******************************************************************************
 * Global Data
 ******************************************************************************/
static Twi* TWI; ///< Singleton


/*******************************************************************************
 * Function Declarations
 ******************************************************************************/
SA_FUNC void twi_reply(TwiAckOpt);

SA_FUNC void twi_stop();

SA_FUNC void twi_release_bus();

SA_FUNC void on_twi_master_tx(uint8_t);

SA_FUNC void on_twi_master_rx(uint8_t);

SA_FUNC void on_twi_slave_tx(uint8_t);

SA_FUNC void on_twi_slave_rx(uint8_t);

SA_FUNC void twi_handle_vect();

SA_FUNC void twi_init(Twi*);

SA_INLINE void twi_disable();

SA_INLINE void twi_begin_tx(const uint8_t);

SA_FUNC TwiBusWriteRes twi_bus_write(uint8_t, uint8_t);

SA_FUNC TwiBusWriteRes twi_end_tx(uint8_t);

SA_FUNC size_t twi_master_write(uint8_t);

SA_FUNC TwiWriteRes twi_slave_write(uint8_t);

SA_FUNC int16_t twi_read();

/**
 * May only be called in:
 *   - slave tx callback
 *   - after twi_begin_tx()
 */
SA_FUNC size_t twi_write(const uint8_t);

SA_FUNC uint8_t twi_bus_read(uint8_t address, uint8_t *data, size_t size,
                             uint8_t send_stop);

SA_FUNC uint8_t twi_bus_request(uint8_t address, size_t size, uint32_t iaddress,
                                size_t isize, uint8_t send_stop);


/*******************************************************************************
 * Function Definitions
 ******************************************************************************/
SA_FUNC void twi_reply(TwiAckOpt send_ack)
{
    if (send_ack == TWI_ACK_SEND) {
        // enable TWI, WI interrupts, ACK, and clear existing interrupt
        TWCR = _BV(TWEA) | _BV(TWEN) | _BV(TWIE) | _BV(TWINT);
    } else {
        // enable TWI, WI interrupts, and clear existing interrupt
        TWCR = _BV(TWEN) | _BV(TWIE) | _BV(TWINT);
    }
}


SA_FUNC void twi_stop()
{
    // enable TWI, WI interrupts, ACK, clear existing interrupt, and send
    // STOP condition
    TWCR = _BV(TWEA) | _BV(TWEN) | _BV(TWIE) | _BV(TWINT) | _BV(TWSTO);

    // TWINT = 0 after this
    loop_until_bit_is_clear(TWCR, TWSTO);
    TWI->state = TWI_READY;
}


SA_FUNC void twi_release_bus()
{
    // enable TWI, WI interrupts, ACK, and clear existing interrupt
    TWCR = _BV(TWEA) | _BV(TWEN) | _BV(TWIE) | _BV(TWINT);
    TWI->state = TWI_READY;
}


SA_FUNC void on_twi_master_tx(const uint8_t status)
{
    switch(status) {
        case TW_MT_SLA_ACK:  // recv address ACK from slave
        case TW_MT_DATA_ACK: // recv data ACK from slave
            if (TWI->master_buff_idx < TWI->master_buff_len) {
                TWDR = TWI->master_buff[TWI->master_buff_idx++];
                twi_reply(TWI_ACK_SEND);
            } else {
                if (TWI->send_stop) {
                    twi_stop();
                } else {
                    TWI->in_repeated_start = 1;
                    TWCR = _BV(TWINT) | _BV(TWSTA) | _BV(TWEN);
                    TWI->state = TWI_READY;
                }
            }
            break;

        case TW_MT_SLA_NACK:
            TWI->error = TW_MT_SLA_NACK;
            twi_stop();
            break;

        case TW_MT_DATA_NACK:
            TWI->error = TW_MT_DATA_NACK;
            twi_stop();
            break;

        case TW_MT_ARB_LOST: // lost bus arbitration
            TWI->error = TW_MT_ARB_LOST;
            twi_release_bus();
            break;
    }
}


SA_FUNC void on_twi_master_rx(const uint8_t status)
{
    switch (status) {
        case TW_MR_DATA_ACK:
            TWI->master_buff[TWI->master_buff_idx++] = TWDR;
            __attribute__((fallthrough));
        case TW_MR_SLA_ACK:
            if (TWI->master_buff_idx < TWI->master_buff_len) {
                twi_reply(TWI_ACK_SEND);
            } else {
                twi_reply(TWI_ACK_DONT_SEND);
            }
            break;
        case TW_MR_DATA_NACK:
            TWI->master_buff[TWI->master_buff_idx++] = TWDR; // last byte
            if (TWI->send_stop) {
                twi_stop();
            } else {
                TWI->in_repeated_start = 1;
                TWCR = _BV(TWINT) | _BV(TWSTA) | _BV(TWEN);
                TWI->state = TWI_READY;
            }
            break;
        case TW_MR_SLA_NACK:
            twi_stop();
            break;
    }
}


SA_FUNC void on_twi_slave_tx(const uint8_t status)
{
    switch (status) {
        case TW_ST_SLA_ACK:
        case TW_ST_ARB_LOST_SLA_ACK:
            TWI->state = TWI_SLAVE_TX;
            TWI->tx_buff_idx = 0;
            TWI->tx_buff_len = 0;
            TWI->on_slave_send();

            if (TWI->tx_buff_len == 0) {
                TWI->tx_buff_len = 1;
                TWI->tx_buff[0] = 0x00;
            }
            __attribute__((fallthrough));

        case TW_ST_DATA_ACK:
            TWDR = TWI->tx_buff[TWI->tx_buff_idx++];

            if (TWI->tx_buff_idx < TWI->tx_buff_len) {
                twi_reply(TWI_ACK_SEND);
            } else {
                twi_reply(TWI_ACK_DONT_SEND);
            }
            break;

        case TW_ST_DATA_NACK:
        case TW_ST_LAST_DATA:
            twi_reply(TWI_ACK_SEND);
            TWI->state = TWI_READY;
            break;
    }
}


SA_FUNC void on_twi_slave_rx(const uint8_t status)
{
    switch (status) {
        case TW_SR_SLA_ACK:
        case TW_SR_GCALL_ACK:
        case TW_SR_ARB_LOST_SLA_ACK:
        case TW_SR_ARB_LOST_GCALL_ACK:
            TWI->state = TWI_SLAVE_RX;
            TWI->rx_buff_idx = 0;
            twi_reply(TWI_ACK_SEND);
            break;

        case TW_SR_DATA_ACK:
        case TW_SR_GCALL_DATA_ACK:
            if (TWI->rx_buff_idx < TWI_BUFF_LEN) {
                TWI->rx_buff[TWI->rx_buff_idx++] = TWDR;
                twi_reply(TWI_ACK_SEND);
            } else {
                twi_reply(TWI_ACK_DONT_SEND);
            }
            break;

        case TW_SR_STOP:
            twi_release_bus();
            if (TWI->rx_buff_idx < TWI_BUFF_LEN) {
                TWI->rx_buff[TWI->rx_buff_idx] = '\0';
            }
            TWI->on_slave_recv(TWI->rx_buff, TWI->rx_buff_idx);
            TWI->rx_buff_idx = 0;
            break;

        case TW_SR_DATA_NACK:
        case TW_SR_GCALL_DATA_NACK:
            twi_reply(TWI_ACK_DONT_SEND);
            break;
    }
}


SA_FUNC void twi_handle_vect()
{
    const uint8_t status = TW_STATUS;

    switch(status) {
        case TW_START:
        case TW_REP_START:
            TWDR = TWI->slave_rw;
            twi_reply(TWI_ACK_SEND);
            break;

        // Master TX
        case TW_MT_SLA_ACK:
        case TW_MT_DATA_ACK:
        case TW_MT_SLA_NACK:
        case TW_MT_DATA_NACK:
        case TW_MT_ARB_LOST:
            on_twi_master_tx(status);
            break;

        // Master RX
        case TW_MR_DATA_ACK:
        case TW_MR_SLA_ACK:
        case TW_MR_DATA_NACK:
        case TW_MR_SLA_NACK:
            on_twi_master_rx(status);
            break;

        // Slave TX
        case TW_ST_SLA_ACK:
        case TW_ST_ARB_LOST_SLA_ACK:
        case TW_ST_DATA_ACK:
        case TW_ST_DATA_NACK:
        case TW_ST_LAST_DATA:
            on_twi_slave_tx(status);
            break;

        // Slave RX
        case TW_SR_SLA_ACK:
        case TW_SR_GCALL_ACK:
        case TW_SR_ARB_LOST_SLA_ACK:
        case TW_SR_ARB_LOST_GCALL_ACK:
        case TW_SR_DATA_ACK:
        case TW_SR_GCALL_DATA_ACK:
        case TW_SR_STOP:
        case TW_SR_DATA_NACK:
        case TW_SR_GCALL_DATA_NACK:
            on_twi_slave_rx(status);
            break;

        case TW_NO_INFO:
            break;

        case TW_BUS_ERROR:
            TWI->error = TW_BUS_ERROR;
            twi_stop();
            break;
    }
}


SA_FUNC void twi_init(Twi* twi)
{
    TWI = twi;

    /* TODO: const members */
    /* if (pinout_null(TWI->pin_sda)) { */
    /*     TWI->pin_sda = (pinout_t) {&PORTC, PORTC4}; */
    /* } */
    /* if (pinout_null(TWI->pin_scl)) { */
    /*     TWI->pin_scl = (pinout_t) {&PORTC, PORTC5}; */
    /* } */

    PRR &= ~_BV(PRTWI); // disable Power Reduction TWI0 (p. 71)

    TWI->state = TWI_READY;
    TWI->send_stop = 1; // default value
    TWI->in_repeated_start = 0;

    pinout_make_pullup_input(TWI->pin_sda);
    pinout_make_pullup_input(TWI->pin_scl);

    /*
     * Set prescaler and bitrate. Formula from p. 267
     */
    TWSR &= ~(_BV(TWPS1) | _BV(TWPS0)); // prescaler: 1
    TWBR = (F_CPU / TWI_SCL_FREQ - 16) / 2;

    // Enable TWI (steal PC4/5), interrupts, and auto-ack
    TWCR = _BV(TWEN) | _BV(TWIE) | _BV(TWEA);
}


SA_INLINE void twi_disable()
{
    TWCR &= ~(_BV(TWEN) | _BV(TWIE) | _BV(TWEA));

    // disable pull-ups
    pinout_clr(TWI->pin_sda);
    pinout_clr(TWI->pin_scl);
}


SA_INLINE void twi_begin_tx(const uint8_t addr)
{
    TWI->tx_addr = addr;
    TWI->is_transmitting = 1;
    TWI->tx_buff_idx = 0;
    TWI->tx_buff_len = 0;
}


SA_FUNC TwiBusWriteRes twi_bus_write(uint8_t wait, uint8_t send_stop)
{
    if (TWI->tx_buff_len >= TWI_BUFF_LEN) {
        return TWI_BUS_WRITE_TOO_LONG;
    }

    while (TWI->state != TWI_READY) // TODO: must be trigged by int.
        ;

    TWI->state = TWI_MASTER_TX;
    TWI->send_stop = send_stop;
    TWI->error = 0xFF; // clear error

    // copy tx_buff to master_buff
    TWI->master_buff_idx = 0;
    TWI->master_buff_len = TWI->tx_buff_len;
    for (uint8_t i = 0; i < TWI->tx_buff_len; ++i) {
        TWI->master_buff[i] = TWI->tx_buff[i];
    }

    // 7-bit address + R/W bit
    TWI->slave_rw = TW_WRITE;
    TWI->slave_rw |= TWI->tx_addr << 1;

    if (TWI->in_repeated_start) {
        // set to false while we wait for the addr byte, just in case an
        // interrupt happens in the meantime
        TWI->in_repeated_start = 0;
        do {
            TWDR = TWI->slave_rw;
        } while(bit_is_set(TWCR, TWWC));

        // enable TWI, WI interrupts, ACK, and clear existing interrupt
        TWCR = _BV(TWEA) | _BV(TWEN) | _BV(TWIE) | _BV(TWINT);
    } else {
        // enable TWI, WI interrupts, ACK, clear existing interrupt, and
        // set START condition
        TWCR = _BV(TWEA) | _BV(TWEN) | _BV(TWIE) | _BV(TWINT) | _BV(TWSTA);
    }

    while(wait && TWI->state == TWI_MASTER_TX)
        ;

    switch (TWI->error) {
        case 0xFF:            return TWI_BUS_WRITE_GOOD;
        case TW_MT_SLA_NACK:  return TWI_BUS_ADDR_NACK;
        case TW_MT_DATA_NACK: return TWI_BUS_DATA_NACK;
        default:              return TWI_BUS_OTHER_ERR;
    }
}


SA_FUNC TwiBusWriteRes twi_end_tx(uint8_t send_stop)
{
    const TwiBusWriteRes ret = twi_bus_write(1, send_stop);
    TWI->is_transmitting = 0;
    TWI->tx_buff_idx = 0;
    TWI->tx_buff_len = 0;
    return ret;
}


SA_FUNC size_t twi_master_write(const uint8_t data)
{
    if (TWI->tx_buff_len >= TWI_BUFF_LEN) {
        return 0;
    }

    TWI->tx_buff[TWI->tx_buff_idx++] = data;
    TWI->tx_buff_len = TWI->tx_buff_idx;
    return 1;
}


SA_FUNC TwiWriteRes twi_slave_write(const uint8_t data)
{
    if (TWI->state != TWI_SLAVE_TX) {
        return TWI_WRITE_NOT_SLAVE;
    }
    if (TWI->tx_buff_len + 1 >= TWI_BUFF_LEN) {
        return TWI_WRITE_BUFF_FULL;
    }

    TWI->tx_buff[TWI->tx_buff_len++] = data;
    return TWI_WRITE_GOOD;
}


SA_FUNC int16_t twi_read()
{
    if (TWI->rx_buff_idx >= TWI->rx_buff_len) {
        return -1;
    }
    return TWI->rx_buff[TWI->rx_buff_idx++];
}


SA_FUNC size_t twi_write(const uint8_t data)
{
    if (TWI->is_transmitting) {
        return twi_master_write(data);
    }
    return twi_slave_write(data) == TWI_WRITE_GOOD ? 1 : 0;
}


SA_FUNC uint8_t twi_bus_read(uint8_t address, uint8_t *data, size_t size,
                             uint8_t send_stop)
{
    if (size >= TWI_BUFF_LEN) {
        return 0;
    }

    while (TWI->state != TWI_READY)
        ;

    TWI->state = TWI_MASTER_RX;
    TWI->send_stop = send_stop;
    TWI->error = 0xFF;

    TWI->master_buff_idx = 0;
    TWI->master_buff_len = size - 1;

    TWI->slave_rw = TW_READ | (address << 1);

    if (TWI->in_repeated_start) {
        TWI->in_repeated_start = 0;

        do {
            TWDR = TWI->slave_rw;
        } while(bit_is_set(TWCR, TWWC));

        // enable TWI, WI interrupts, ACK, and clear existing interrupt
        TWCR = _BV(TWEA) | _BV(TWEN) | _BV(TWIE) | _BV(TWINT);
    } else {
        // enable TWI, WI interrupts, ACK, clear existing interrupt, and set
        // START condition
        TWCR = _BV(TWEA) | _BV(TWEN) | _BV(TWIE) | _BV(TWINT) | _BV(TWSTA);
    }

    while (TWI->state == TWI_MASTER_RX)
        ;

    if (TWI->master_buff_idx < size) {
        size = TWI->master_buff_idx;
    }

    for (uint8_t i = 0; i < size; ++i) {
        data[i] = TWI->master_buff[i];
    }

    return size;
}


SA_FUNC uint8_t twi_bus_request(uint8_t address, size_t size, uint32_t iaddress,
                                size_t isize, uint8_t send_stop)
{
    if (isize > 0) {
        twi_begin_tx(address);

        if (isize > 3) {
            isize = 3; // max size
        }

        while (isize--) {
            twi_write(iaddress >> (isize * 8));
        }
        twi_end_tx(0);
    }

    if (size > TWI_BUFF_LEN) {
        size = TWI_BUFF_LEN;
    }

    const uint8_t read = twi_bus_read(address, TWI->rx_buff, size, send_stop);
    TWI->rx_buff_idx = 0;
    TWI->rx_buff_len = size;
    return read;
}
#endif//SANGSTER_TWI_H
