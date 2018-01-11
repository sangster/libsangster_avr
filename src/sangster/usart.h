#ifndef SANGSTER_USART_H
#define SANGSTER_USART_H

#include <avr/io.h>
/* #include <util/delay.h> */

void usart_init(const uint16_t baud);

uint8_t usart_recv();

#endif // SANGSTER_USART_H
