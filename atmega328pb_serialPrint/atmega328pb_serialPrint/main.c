/*
 * atmega328pb_serialPrint.c
 *
 * Created: 2/12/2024 8:46:38 PM
 * Author : nmcgill
 */

#include "uart.h"
#include <avr/io.h>

#define F_CPU                16000000UL   // 16MHz clock
#define USART_BAUD_RATE      9600
#define USART_BAUD_PRESCALER (((F_CPU / (USART_BAUD_RATE * 16UL))) - 1)

int
main(void) {
    // Set up serial UART printing
    UART_init(USART_BAUD_PRESCALER);
    UART_putstring("ATmega328PB - Serial UART Printing Example\r\n");

    // Main loop
    while (1)
        ;
}
