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
uint8_t sd_card_init(SdCard* card, uint8_t sck_rate_id);

uint8_t sd_card_card_command(SdCard* card, uint8_t cmd, uint32_t arg);

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
void sd_card_partial_block_read(SdCard* card, uint8_t value);

void sd_card_read_end(SdCard* card);

uint8_t sd_card_wait_not_busy(uint16_t timeout_millis);

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
uint8_t sd_card_set_sck_rate(SdCard* card, uint8_t sck_rate_id);

/**
 * Read a 512 byte block from an SD card device.
 *
 * @param[in] block Logical block to be read.
 * @param[out] dst Pointer to the location that will receive the data.

 * @return The value one, true, is returned for success and the value zero,
 *   false, is returned for failure.
 */
uint8_t sd_card_read_block(SdCard* card, uint32_t block, uint8_t* dst);

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
uint8_t sd_card_read_data(SdCard* card, uint32_t block, uint16_t offset,
                          uint16_t count, uint8_t* dst);


/** Wait for start block token */
uint8_t sd_card_wait_start_block(SdCard* card);

/**
 * Writes a 512 byte block to an SD card.
 *
 * @param[in] block_number Logical block to be written.
 * @param[in] src Pointer to the location of the data to be written.
 * @return The value one, true, is returned for success and the value zero,
 *   false, is returned for failure.
 */
uint8_t sd_card_write_block(SdCard* card, uint32_t block_number,
                            const uint8_t* src);


uint8_t sd_card_write_data_seq(SdCard* card, const uint8_t* src);


// send one block of data for write block or write multiple blocks
uint8_t sd_card_write_data(SdCard* card, uint8_t token, const uint8_t* src);


/**
 * Determine the size of an SD flash memory card.
 *
 * @return The number of 512 byte data blocks in the card or zero if an error
 *   occurs.
 */
uint32_t sd_card_size(SdCard* card);


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
uint8_t sd_card_erase(SdCard* card, uint32_t first_block, uint32_t last_block);


/**
 * Determine if card supports single block erase.
 *
 * @return The value one, true, is returned if single block erase is supported.
 *   The value zero, false, is returned if single block erase is not supported.
 */
uint8_t sd_card_erase_single_block_enable(SdCard* card);


/** read CID or CSR register */
uint8_t sd_card_read_register(SdCard* card, uint8_t cmd, uint8_t* dst);


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
uint8_t sd_card_write_start(SdCard* card, uint32_t block_number,
                            uint32_t erase_count);


/**
 * End a write multiple blocks sequence.
 *
 * @return The value one, true, is returned for success and the value zero,
 *   false, is returned for failure.
 */
uint8_t sd_card_write_stop(SdCard* card);


uint8_t sd_card_read_cid(SdCard* card, SdCid* cid);


uint8_t sd_card_read_csd(SdCard* card, SdCsd* csd);

#endif//SD_CARD_H
