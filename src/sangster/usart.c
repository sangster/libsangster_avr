// vim: expandtab:sw=4:
#include "usart.h"

#ifndef BAUD_TOL
#  define BAUD_TOL 2
#endif

void usart_init(const uint16_t baud)
{
    uint8_t use_2x = 0;
    uint16_t ubrr_value = ((F_CPU) + 8UL * (baud)) / (16UL * (baud)) -1UL;

    if (100 * F_CPU > (16 * (ubrr_value + 1)) * (100 * baud + baud * BAUD_TOL)) {
      use_2x = 1;
    } else if (100 * F_CPU < (16 * (ubrr_value + 1)) * (100 * baud - baud * BAUD_TOL)) {
      use_2x = 1;
    }

    if (use_2x) {
        ubrr_value = (F_CPU + 4UL * baud) / (8UL * baud) - 1UL;
        UCSR0A |= _BV(U2X0);
    } else {
        UCSR0A &= ~(_BV(U2X0));
    }

    // baud rate
    UBRR0H = ubrr_value & 0xff;
    UBRR0L = ubrr_value >> 8;

    /* UCSR0B = _BV(RXEN0); // enable recv */
    /* UCSR0C = _BV(UCSZ00) | _BV(UCSZ01); // frame format: 8bit */
}

uint8_t usart_recv()
{
    loop_until_bit_is_set(UCSR0A, RXC0);
    return UDR0;
}
