/* Arduino SdFat Library
 * Copyright (C) 2009 by William Greiman
 *
 * This file is part of the Arduino SdFat Library
 *
 * This Library is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * This Library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * the Arduino SdFat Library. If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef SD_CARD_H
#define SD_CARD_H
/**
 * @file
 */
#include <stdbool.h>
#include <avr/cpufunc.h>
#include <util/atomic.h>
#include "sangster/api.h"
#include "sangster/pinout.h"
#include "sangster/timer0.h"
#include "sangster/sd/fat_structs.h"

// SD card commands

/** GO_IDLE_STATE - init card in spi mode if CS low */
#define CMD0 0x00

/** SEND_IF_COND - verify SD Memory Card interface operating condition.*/
#define CMD8 0x08

/** SEND_CSD - read the Card Specific Data (CSD register) */
#define CMD9 0x09

/** SEND_CID - read the card identification information (CID register) */
#define CMD10 0x0A

/** SEND_STATUS - read the card status register */
#define CMD13 0x0D

/** READ_BLOCK - read a single data block from the card */
#define CMD17 0x11

/** WRITE_BLOCK - write a single data block to the card */
#define CMD24 0x18

/** WRITE_MULTIPLE_BLOCK - write blocks of data until a STOP_TRANSMISSION */
#define CMD25 0x19

/** ERASE_WR_BLK_START - sets the address of the first block to be erased */
#define CMD32 0x20

/** ERASE_WR_BLK_END - sets the address of the last block of the continuous
    range to be erased*/
#define CMD33 0x21

/** ERASE - erase all previously selected blocks */
#define CMD38 0x26

/** APP_CMD - escape for application specific command */
#define CMD55 0x37

/** READ_OCR - read the OCR register of a card */
#define CMD58 0x3A

/** SET_WR_BLK_ERASE_COUNT - Set the number of write blocks to be
     pre-erased before writing */
#define ACMD23 0x17

/** SD_SEND_OP_COMD - Sends host capacity support information and
    activates the card's initialization process */
#define ACMD41 0x29


//------------------------------------------------------------------------------
/** status for card in the ready state */
#define R1_READY_STATE 0x00

/** status for card in the idle state */
#define R1_IDLE_STATE 0x01

/** status bit for illegal command */
#define R1_ILLEGAL_COMMAND 0x04

/** start data token for read or write single block*/
#define DATA_START_BLOCK 0xFE

/** stop token for write multiple blocks*/
#define STOP_TRAN_TOKEN 0xFD

/** start data token for write multiple blocks*/
#define WRITE_MULTIPLE_TOKEN 0xFC

/** mask for data response tokens after a write block operation */
#define DATA_RES_MASK 0x1F

/** write data accepted token */
#define DATA_RES_ACCEPTED 0x05


/** Set SCK to max rate of F_CPU/2. See sd_card_set_sck_rate(). */
#define SPI_FULL_SPEED 0

/** Set SCK rate to F_CPU/4. See sd_card_set_sck_rate(). */
#define SPI_HALF_SPEED 1

/** Set SCK rate to F_CPU/8. sd_card_set_sck_rate(). */
#define SPI_QUARTER_SPEED 2

/** init timeout ms */
#define SD_INIT_TIMEOUT ((uint16_t) 2000)

/** erase timeout ms */
#define SD_ERASE_TIMEOUT ((uint16_t) 10000)

/** read timeout ms */
#define SD_READ_TIMEOUT ((uint16_t) 300)

/** write time out ms */
#define SD_WRITE_TIMEOUT ((uint16_t) 600)

// SD card errors
/** timeout error for command CMD0 */
#define SD_CARD_ERROR_CMD0 0x1

/** CMD8 was not accepted - not a valid SD card*/
#define SD_CARD_ERROR_CMD8 0x2

/** card returned an error response for CMD17 (read block) */
#define SD_CARD_ERROR_CMD17 0x3

/** card returned an error response for CMD24 (write block) */
#define SD_CARD_ERROR_CMD24 0x4

/**  WRITE_MULTIPLE_BLOCKS command failed */
#define SD_CARD_ERROR_CMD25 0x05

/** card returned an error response for CMD58 (read OCR) */
#define SD_CARD_ERROR_CMD58 0x06

/** SET_WR_BLK_ERASE_COUNT failed */
#define SD_CARD_ERROR_ACMD23 0x07

/** card's ACMD41 initialization process timeout */
#define SD_CARD_ERROR_ACMD41 0x08

/** card returned a bad CSR version field */
#define SD_CARD_ERROR_BAD_CSD 0x09

/** erase block group command failed */
#define SD_CARD_ERROR_ERASE 0x0A

/** card not capable of single block erase */
#define SD_CARD_ERROR_ERASE_SINGLE_BLOCK 0x0B

/** Erase sequence timed out */
#define SD_CARD_ERROR_ERASE_TIMEOUT 0x0C

/** card returned an error token instead of read data */
#define SD_CARD_ERROR_READ 0x0D

/** read CID or CSD failed */
#define SD_CARD_ERROR_READ_REG 0x0E

/** timeout while waiting for start of read data */
#define SD_CARD_ERROR_READ_TIMEOUT 0x0F

/** card did not accept STOP_TRAN_TOKEN */
#define SD_CARD_ERROR_STOP_TRAN 0x10

/** card returned an error token as a response to a write operation */
#define SD_CARD_ERROR_WRITE 0x11

/** attempt to write protected block zero */
#define SD_CARD_ERROR_WRITE_BLOCK_ZERO 0x12

/** card did not go ready for a multiple block write */
#define SD_CARD_ERROR_WRITE_MULTIPLE 0x13

/** card returned an error to a CMD13 status check after a write */
#define SD_CARD_ERROR_WRITE_PROGRAMMING 0x14

/** timeout occurred during write programming */
#define SD_CARD_ERROR_WRITE_TIMEOUT 0x15

/** incorrect rate selected */
#define SD_CARD_ERROR_SCK_RATE 0x16

// card types
/** Standard capacity V1 SD card */
#define SD_CARD_TYPE_SD1 1

/** Standard capacity V2 SD card */
#define SD_CARD_TYPE_SD2 2

/** High Capacity SD card */
#define SD_CARD_TYPE_SDHC 3


typedef struct sd_card SdCard;
struct sd_card
{
    uint32_t block;
    Pinout chip_select_pin;
    Pinout mosi_pin;
    Pinout miso_pin;
    Pinout clock_pin;
    uint8_t error_code;
    uint8_t in_block;
    uint16_t offset;
    uint8_t partial_block_read;
    uint8_t status;
    uint8_t type;
    uint8_t write_crc;
};


/** Skip remaining data in a block when in partial block read mode. */
SA_FUNC void sd_card_read_end(SdCard* card)
{
    if (card->in_block) {
        // skip data and crc
        // optimize skip for hardware
        SPDR = 0xFF;
        while (card->offset++ < 513) {
            while (!(SPSR & _BV(SPIF)))
                ;
            SPDR = 0xFF;
        }
        // wait for last crc byte
        while (!(SPSR & _BV(SPIF)))
            ;
        pinout_set(card->chip_select_pin);
        card->in_block = 0;
    }
}


/** Send a byte to the card */
SA_INLINE void spi_send(uint8_t b)
{
    SPDR = b;
    loop_until_bit_is_set(SPSR, SPIF);
}


SA_INLINE uint8_t spi_rec()
{
    spi_send(0xFF);
    return SPDR;
}


// wait for card to go not busy
SA_FUNC uint8_t sd_card_wait_not_busy(uint16_t timeout_millis)
{
    const uint16_t t0 = timer0_ms();

    do {
        if (spi_rec() == 0xFF) {
            return true;
        }
    }
    while (timer0_ms() - t0 < timeout_millis)
        ;
    return false;
}


// send command and return error code.  Return zero for OK
SA_FUNC uint8_t sd_card_card_command(SdCard* card, uint8_t cmd, uint32_t arg)
{
    sd_card_read_end(card);            // end read if in partial_block_read mode
    pinout_clr(card->chip_select_pin); // select card
    sd_card_wait_not_busy(300);        // wait up to 300 ms if busy
    spi_send(cmd | 0x40);              // send command

    // send argument
    for (int8_t s = 24; s >= 0; s -= 8) {
        spi_send(arg >> s);
    }

    // send CRC
    uint8_t crc = 0xFF;
    if (cmd == CMD0) {
        crc = 0x95;  // correct crc for CMD0 with arg 0
    }
    if (cmd == CMD8) {
        crc = 0x87;  // correct crc for CMD8 with arg 0x1AA
    }
    spi_send(crc);

    // wait for response
    for (uint8_t i = 0; ((card->status = spi_rec()) & 0x80) && i != 0xFF; i++)
        ;
    return card->status;
}


SA_INLINE uint8_t app_command(SdCard* card, uint8_t cmd, uint32_t arg)
{
    sd_card_card_command(card, CMD55, 0);
    return sd_card_card_command(card, cmd, arg);
}


/**
 * Set the SPI clock rate.
 *
 * @param[in] sckRateID A value in the range [0, 6].
 *
 * The SPI clock will be set to F_CPU/pow(2, 1 + sckRateID). The maximum SPI
 * rate is F_CPU/2 for \a sckRateID = 0 and the minimum rate is F_CPU/128 for
 * \a scsRateID = 6.
 *
 * @return The value one, true, is returned for success and the value zero,
 *   false, is returned for an invalid value of \a sckRateID.
 */
SA_FUNC uint8_t sd_card_set_sck_rate(SdCard* card, uint8_t sck_rate_id)
{
    if (sck_rate_id > 6) {
        card->error_code = SD_CARD_ERROR_SCK_RATE;
        return false;
    }

    // see avr processor datasheet for SPI register bit definitions
    if ((sck_rate_id & 0x01) || sck_rate_id == 6) {
        SPSR &= ~_BV(SPI2X);
    } else {
        SPSR |= _BV(SPI2X);
    }
    SPCR &= ~(_BV(SPR1) | _BV(SPR0));
    SPCR |= (sck_rate_id & 0x04 ? _BV(SPR1) : 0)
          | (sck_rate_id & 0x02 ? _BV(SPR0) : 0);
    return true;
}


/**
 * Initialize an SD flash memory card.
 *
 * @param[in] sckRateID SPI clock rate selector. See setSckRate().
 * @param[in] chipSelectPin SD chip select pin number.
 *
 * @return The value one, true, is returned for success and the value zero,
 *   false, is returned for failure.  The reason for failure can be determined by
 *   calling errorCode() and errorData().
 */
SA_FUNC uint8_t sd_card_init(SdCard* card, uint8_t sck_rate_id)
{
    card->write_crc = 0;
    card->error_code = 0;
    card->in_block = 0;
    card->partial_block_read = 0;
    card->type = 0;

    timer0_start();

    // 16-bit init start time allows over a minute
    uint16_t t0 = timer0_ms();
    uint32_t arg;

    // set pin modes
    pinout_make_output(card->chip_select_pin);
    pinout_set(card->chip_select_pin);

    pinout_make_input(card->miso_pin);
    pinout_make_output(card->mosi_pin);
    pinout_make_output(card->clock_pin);

    // SS must be in output mode even it is not chip select
    pinout_make_output(card->chip_select_pin);

    // disable any SPI device using hardware SS pin
    pinout_set(card->chip_select_pin);

    // Enable SPI, Master, clock rate f_osc/128
    SPCR = _BV(SPE) | _BV(MSTR) | _BV(SPR1) | _BV(SPR0);
    SPSR &= ~_BV(SPI2X); // clear double speed

    // must supply min of 74 clock cycles with CS high.
    for (uint8_t i = 0; i < 10; ++i) {
        spi_send(0xFF);
    }

    pinout_clr(card->chip_select_pin);

    // command to go idle in SPI mode
    while ((card->status = sd_card_card_command(card, CMD0, 0)) != R1_IDLE_STATE) {
        if ((timer0_ms() - t0) > SD_INIT_TIMEOUT) {
            card->error_code = SD_CARD_ERROR_CMD0;
            goto fail;
        }
    }

    // check SD version
    if ((sd_card_card_command(card, CMD8, 0x1AA) & R1_ILLEGAL_COMMAND)) {
        card->type = SD_CARD_TYPE_SD1;
    } else {
        // only need last byte of r7 response
        for (uint8_t i = 0; i < 4; ++i) {
            card->status = spi_rec();
        }
        if (card->status != 0xAA) {
            card->error_code = SD_CARD_ERROR_CMD8;
            goto fail;
        }
        card->type = SD_CARD_TYPE_SD2;
    }

    // initialize card and send host supports SDHC if SD2
    arg = card->type == SD_CARD_TYPE_SD2 ? 0x40000000 : 0;

    while ((card->status = app_command(card, ACMD41, arg)) != R1_READY_STATE) {
        // check for timeout
        if ((timer0_ms() - t0) > SD_INIT_TIMEOUT) {
            card->error_code = SD_CARD_ERROR_ACMD41;
            goto fail;
        }
    }

    // if SD2 read OCR register to check for SDHC card
    if (card->type == SD_CARD_TYPE_SD2) {
        if (sd_card_card_command(card, CMD58, 0)) {
            card->error_code = SD_CARD_ERROR_CMD58;
            goto fail;
        }
        if ((spi_rec() & 0xC0) == 0xC0) {
            card->type = SD_CARD_TYPE_SDHC;
        }
        // discard rest of ocr - contains allowed voltage range
        for (uint8_t i = 0; i < 3; ++i) {
            spi_rec();
        }
    }

    pinout_set(card->chip_select_pin);
    return sd_card_set_sck_rate(card, sck_rate_id);

fail:
    pinout_set(card->chip_select_pin);
    return false;
}


/**
 * Enable or disable partial block reads.
 *
 * Enabling partial block reads improves performance by allowing a block to be
 * read over the SPI bus as several sub-blocks.  Errors may occur if the time
 * between reads is too long since the SD card may timeout.  The SPI SS line
 * will be held low until the entire block is read or read_end() is called.
 *
 * Use this for applications like the Adafruit Wave Shield.
 *
 * @param[in] value The value TRUE (non-zero) or FALSE (zero).)
 */
SA_FUNC void sd_card_partial_block_read(SdCard* card, uint8_t value)
{
    sd_card_read_end(card);
    card->partial_block_read = value;
}


/** Wait for start block token */
SA_FUNC uint8_t sd_card_wait_start_block(SdCard* card)
{
    timer0_reset();
    uint16_t t0 = timer0_ms();
    while ((card->status = spi_rec()) == 0xFF) {
        if (timer0_ms() - t0 > SD_READ_TIMEOUT) {
            card->error_code = SD_CARD_ERROR_READ_TIMEOUT;
            goto fail;
        }
    }
    if (card->status != DATA_START_BLOCK) {
        card->error_code = SD_CARD_ERROR_READ;
        goto fail;
    }
    return true;

fail:
    pinout_set(card->chip_select_pin);
    return false;
}


/**
 * Read part of a 512 byte block from an SD card.
 *
 * @param[in] block Logical block to be read.
 * @param[in] offset Number of bytes to skip at start of block
 * @param[out] dst Pointer to the location that will receive the data.
 * @param[in] count Number of bytes to read
 * @return The value one, true, is returned for success and the value zero,
 *   false, is returned for failure.
 */
SA_FUNC uint8_t sd_card_read_data(SdCard* card, uint32_t block, uint16_t offset,
                                  uint16_t count, uint8_t* dst)
{
    uint16_t n;
    if (count == 0) {
        return true;
    }
    if (count + offset > 512) {
        goto fail;
    }
    if (!card->in_block || block != card->block || offset < card->offset) {
        card->block = block;
        // use address if not SDHC card
        if (card->type != SD_CARD_TYPE_SDHC) {
            block <<= 9;
        }
        if (sd_card_card_command(card, CMD17, block)) {
            card->error_code = SD_CARD_ERROR_CMD17;
            goto fail;
        }
        if (!sd_card_wait_start_block(card)) {
            goto fail;
        }
        card->offset = 0;
        card->in_block = 1;
    }

    SPDR = 0xFF; // start first spi transfer

    // skip data before offset
    for ( ; card->offset < offset; card->offset++) {
        loop_until_bit_is_set(SPSR, SPIF);
        SPDR = 0xFF;
    }
    // transfer data
    n = count - 1;
    for (uint16_t i = 0; i < n; i++) {
        loop_until_bit_is_set(SPSR, SPIF);
        dst[i] = SPDR;
        SPDR = 0xFF;
    }
    loop_until_bit_is_set(SPSR, SPIF); // wait for last byte
    dst[n] = SPDR;

    card->offset += count;
    if (!card->partial_block_read || card->offset >= 512) {
        // read rest of data, checksum and set chip select high
        sd_card_read_end(card);
    }
    return true;

fail:
    pinout_set(card->chip_select_pin);
    return false;
}


/**
 * Read a 512 byte block from an SD card device.
 *
 * @param[in] block Logical block to be read.
 * @param[out] dst Pointer to the location that will receive the data.

 * @return The value one, true, is returned for success and the value zero,
 *   false, is returned for failure.
 */
SA_INLINE uint8_t sd_card_read_block(SdCard* card, uint32_t block,
                                         uint8_t* dst)
{
    return sd_card_read_data(card, block, 0, 512, dst);
}


// send one block of data for write block or write multiple blocks
SA_FUNC uint8_t sd_card_write_data(SdCard* card, uint8_t token,
                                  const uint8_t* src)
{
    // CRC16 checksum is supposed to be ignored in SPI mode (unless explicitly
    // enabled) and a dummy value is normally written.  A few funny cards (e.g.
    // Eye-Fi X2) expect a valid CRC anyway.  Call setCRC(true) to enable CRC16
    // checksum on block writes.  This has a noticeable impact on write speed.
    // :(
    int16_t crc;
    if(card->write_crc) {
        int16_t i, x;
        // CRC16 code via Scott Dattalo www.dattalo.com
        for(crc = i = 0; i < 512; i++) {
            x   = ((crc >> 8) ^ src[i]) & 0xff;
            x  ^= x >> 4;
            crc = (crc << 8) ^ (x << 12) ^ (x << 5) ^ x;
        }
    } else {
        crc = 0xffff; // Dummy CRC value
    }

    SPDR = token; // send data - optimized loop

    // send two byte per iteration
    for (uint16_t i = 0; i < 512; i += 2) {
        while (!(SPSR & _BV(SPIF)))
            ;
        SPDR = src[i];
        while (!(SPSR & _BV(SPIF)))
            ;
        SPDR = src[i + 1];
    }

    // wait for last data byte
    while (!(SPSR & _BV(SPIF)))
        ;

    spi_send(crc >> 8); // Might be dummy value, that's OK
    spi_send(crc);

    card->status = spi_rec();
    if ((card->status & DATA_RES_MASK) != DATA_RES_ACCEPTED) {
        card->error_code = SD_CARD_ERROR_WRITE;
        pinout_set(card->chip_select_pin);
        return false;
    }
    return true;
}


/**
 * Writes a 512 byte block to an SD card.
 *
 * @param[in] block_number Logical block to be written.
 * @param[in] src Pointer to the location of the data to be written.
 * @return The value one, true, is returned for success and the value zero,
 *   false, is returned for failure.
 */
SA_FUNC uint8_t sd_card_write_block(SdCard* card, uint32_t block_number,
                                   const uint8_t* src)
{
    // use address if not SDHC card
    if (card->type != SD_CARD_TYPE_SDHC) {
        block_number <<= 9;
    }
    if (sd_card_card_command(card, CMD24, block_number)) {
        card->error_code = SD_CARD_ERROR_CMD24;
        goto fail;
    }
    if (!sd_card_write_data(card, DATA_START_BLOCK, src)) {
        goto fail;
    }

    // wait for flash programming to complete
    if (!sd_card_wait_not_busy(SD_WRITE_TIMEOUT)) {
        card->error_code = SD_CARD_ERROR_WRITE_TIMEOUT;
        goto fail;
    }
    // response is r2 so get and check two bytes for nonzero
    if (sd_card_card_command(card, CMD13, 0) || spi_rec()) {
        card->error_code = SD_CARD_ERROR_WRITE_PROGRAMMING;
        goto fail;
    }
    pinout_set(card->chip_select_pin);
    return true;

fail:
    pinout_set(card->chip_select_pin);
    return false;
}


/** Write one data block in a multiple block write sequence */
SA_FUNC uint8_t sd_card_write_data_seq(SdCard* card, const uint8_t* src)
{
    // wait for previous write to finish
    if (!sd_card_wait_not_busy(SD_WRITE_TIMEOUT)) {
        card->error_code = SD_CARD_ERROR_WRITE_MULTIPLE;
        pinout_set(card->chip_select_pin);
        return false;
    }
    return sd_card_write_data(card, WRITE_MULTIPLE_TOKEN, src);
}


/** read CID or CSR register */
SA_FUNC uint8_t sd_card_read_register(SdCard* card, uint8_t cmd, uint8_t* dst)
{
    if (sd_card_card_command(card, cmd, 0)) {
        card->error_code = SD_CARD_ERROR_READ_REG;
        goto fail;
    }
    if (!sd_card_wait_start_block(card)) {
        goto fail;
    }
    // transfer data
    for (uint16_t i = 0; i < 16; i++) {
        dst[i] = spi_rec();
    }
    spi_rec(); // get first crc byte
    spi_rec(); // get second crc byte

    pinout_set(card->chip_select_pin);
    return true;

fail:
    pinout_set(card->chip_select_pin);
    return false;
}


SA_INLINE uint8_t sd_card_read_cid(SdCard* card, SdCid* cid)
{
    return sd_card_read_register(card, CMD10, (uint8_t*) cid);
}


SA_INLINE uint8_t sd_card_read_csd(SdCard* card, SdCsd* csd)
{
    return sd_card_read_register(card, CMD9, (uint8_t*) csd);
}


/**
 * Determine the size of an SD flash memory card.
 *
 * @return The number of 512 byte data blocks in the card or zero if an error
 *   occurs.
 */
SA_FUNC uint32_t sd_card_size(SdCard* card)
{
    SdCsd csd;
    if (!sd_card_read_csd(card, &csd)) {
        return 0;
    }
    if (csd.v1.csd_ver == 0) {
        uint8_t read_bl_len = csd.v1.read_bl_len;
        uint16_t c_size = (csd.v1.c_size_high << 10)
            | (csd.v1.c_size_mid << 2) | csd.v1.c_size_low;
        uint8_t c_size_mult = (csd.v1.c_size_mult_high << 1)
            | csd.v1.c_size_mult_low;
        return (uint32_t) (c_size + 1) << (c_size_mult + read_bl_len - 7);
    } else if (csd.v2.csd_ver == 1) {
        uint32_t c_size = ((uint32_t) csd.v2.c_size_high << 16)
            | (csd.v2.c_size_mid << 8) | csd.v2.c_size_low;
        return (c_size + 1) << 10;
    } else {
        card->error_code = SD_CARD_ERROR_BAD_CSD;
        return 0;
    }
}


/**
 * Determine if card supports single block erase.
 *
 * @return The value one, true, is returned if single block erase is supported.
 *   The value zero, false, is returned if single block erase is not supported.
 */
SA_INLINE uint8_t sd_card_erase_single_block_enable(SdCard* card)
{
  SdCsd csd;
  return sd_card_read_csd(card, &csd) ? csd.v1.erase_blk_en : 0;
}


/**
 * Erase a range of blocks.
 *
 * @param[in] first_block The address of the first block in the range.
 * @param[in] last_block The address of the last block in the range.
 *
 * @note This function requests the SD card to do a flash erase for a range of
 *   blocks. The data on the card after an erase operation is either 0 or 1,
 *   depends on the card vendor. The card must support single block erase.
 *
 * @return The value one, true, is returned for success and the value zero,
 *   false, is returned for failure.
 */
SA_FUNC uint8_t sd_card_erase(SdCard* card, uint32_t first_block,
                             uint32_t last_block)
{
    if (!sd_card_erase_single_block_enable(card)) {
        card->error_code = SD_CARD_ERROR_ERASE_SINGLE_BLOCK;
        goto fail;
    }
    if (card->type != SD_CARD_TYPE_SDHC) {
        first_block <<= 9;
        last_block <<= 9;
    }
    if (sd_card_card_command(card, CMD32, first_block)
            || sd_card_card_command(card, CMD33, last_block)
            || sd_card_card_command(card, CMD38, 0)) {
        card->error_code = SD_CARD_ERROR_ERASE;
        goto fail;
    }
    if (!sd_card_wait_not_busy(SD_ERASE_TIMEOUT)) {
        card->error_code = SD_CARD_ERROR_ERASE_TIMEOUT;
        goto fail;
    }

    pinout_set(card->chip_select_pin);
    return true;

fail:
    pinout_set(card->chip_select_pin);
    return false;
}


/**
 * Start a write multiple blocks sequence.
 *
 * @param[in] block_number Address of first block in sequence.
 * @param[in] erase_count The number of blocks to be pre-erased.
 *
 * @note This function is used with writeData() and writeStop()
 *   for optimized multiple block writes.
 *
 * @return The value one, true, is returned for success and the value zero,
 *   false, is returned for failure.
 */
SA_FUNC uint8_t sd_card_write_start(SdCard* card, uint32_t block_number,
                                   uint32_t erase_count)
{
    // send pre-erase count
    if (app_command(card, ACMD23, erase_count)) {
        card->error_code = SD_CARD_ERROR_ACMD23;
        goto fail;
    }
    // use address if not SDHC card
    if (card->type != SD_CARD_TYPE_SDHC) {
        block_number <<= 9;
    }
    if (sd_card_card_command(card, CMD25, block_number)) {
        card->error_code = SD_CARD_ERROR_CMD25;
        goto fail;
    }

    return true;

fail:
    pinout_set(card->chip_select_pin);
    return false;
}


/**
 * End a write multiple blocks sequence.
 *
 * @return The value one, true, is returned for success and the value zero,
 *   false, is returned for failure.
 */
SA_FUNC uint8_t sd_card_write_stop(SdCard* card)
{
    if (!sd_card_wait_not_busy(SD_WRITE_TIMEOUT)) {
        goto fail;
    }
    spi_send(STOP_TRAN_TOKEN);
    if (!sd_card_wait_not_busy(SD_WRITE_TIMEOUT)) {
        goto fail;
    }

    pinout_set(card->chip_select_pin);
    return true;

fail:
    card->error_code = SD_CARD_ERROR_STOP_TRAN;
    pinout_set(card->chip_select_pin);
    return false;
}
#endif//SD_CARD_H
