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
#include "sd_card.h"

static inline uint8_t app_command(SdCard* card, uint8_t cmd, uint32_t arg)
{
    sd_card_card_command(card, CMD55, 0);
    return sd_card_card_command(card, cmd, arg);
}


/*
 * functions for hardware SPI
 */
/** Send a byte to the card */
static void spi_send(uint8_t b)
{
    SPDR = b;
    loop_until_bit_is_set(SPSR, SPIF);
}


static uint8_t spi_rec()
{
    spi_send(0xFF);
    return SPDR;
}

uint8_t sd_card_init(SdCard* card, uint8_t sck_rate_id)
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


// send command and return error code.  Return zero for OK
uint8_t sd_card_card_command(SdCard* card, uint8_t cmd, uint32_t arg)
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


void sd_card_partial_block_read(SdCard* card, uint8_t value)
{
    sd_card_read_end(card);
    card->partial_block_read = value;
}


/** Skip remaining data in a block when in partial block read mode. */
void sd_card_read_end(SdCard* card)
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


// wait for card to go not busy
uint8_t sd_card_wait_not_busy(uint16_t timeout_millis)
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


uint8_t sd_card_set_sck_rate(SdCard* card, uint8_t sck_rate_id)
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


uint8_t sd_card_read_block(SdCard* card, uint32_t block, uint8_t* dst)
{
    return sd_card_read_data(card, block, 0, 512, dst);
}


uint8_t sd_card_read_data(SdCard* card, uint32_t block, uint16_t offset,
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


uint8_t sd_card_wait_start_block(SdCard* card)
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


uint8_t sd_card_write_block(SdCard* card, uint32_t block_number,
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
uint8_t sd_card_write_data_seq(SdCard* card, const uint8_t* src)
{
    // wait for previous write to finish
    if (!sd_card_wait_not_busy(SD_WRITE_TIMEOUT)) {
        card->error_code = SD_CARD_ERROR_WRITE_MULTIPLE;
        pinout_set(card->chip_select_pin);
        return false;
    }
    return sd_card_write_data(card, WRITE_MULTIPLE_TOKEN, src);
}


// send one block of data for write block or write multiple blocks
uint8_t sd_card_write_data(SdCard* card, uint8_t token, const uint8_t* src)
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
 * Determine the size of an SD flash memory card.
 *
 * @return The number of 512 byte data blocks in the card or zero if an error
 *   occurs.
 */
uint32_t sd_card_size(SdCard* card)
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


uint8_t sd_card_erase(SdCard* card, uint32_t first_block,
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


uint8_t sd_card_erase_single_block_enable(SdCard* card)
{
  SdCsd csd;
  return sd_card_read_csd(card, &csd) ? csd.v1.erase_blk_en : 0;
}


uint8_t sd_card_read_register(SdCard* card, uint8_t cmd, uint8_t* dst)
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


uint8_t sd_card_write_stop(SdCard* card)
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


uint8_t sd_card_read_cid(SdCard* card, SdCid* cid)
{
    return sd_card_read_register(card, CMD10, (uint8_t*) cid);
}


uint8_t sd_card_read_csd(SdCard* card, SdCsd* csd)
{
    return sd_card_read_register(card, CMD9, (uint8_t*) csd);
}
