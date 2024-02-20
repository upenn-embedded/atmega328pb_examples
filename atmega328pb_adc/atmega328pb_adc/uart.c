#include "uart.h"
#include <avr/io.h>

// See Section 24.6 "USART Initialization" of the ATmega328PB datasheet
// Sets up serial UART on USART0, pins PD0 (RXD0) and PD1 (TXD0)

void
UART_init(int usart_baud_rate) {

    // Set baud rate
    UBRR0H = (unsigned char) (usart_baud_rate >> 8);
    UBRR0L = (unsigned char) usart_baud_rate;
    // Enable receiver and transmitter
    UCSR0B = (1 << RXEN0) | (1 << TXEN0);
    // Enable as Asynchronous USART
    UCSR0C &= ~((1 << UMSEL01) | (1 << UMSEL00));
    /* Set frame format for 8N1: 8 data bits, no parity, 1 stop bit */
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);     // 8 data bits
    UCSR0C &= ~((1 << UPM01) & (1 << UPM00));   // Disable Parity
    UCSR0C &= ~(1 << USBS0);                    // 1 stop bit
}

void
UART_send(unsigned char data) {
    // Wait for empty transmit buffer
    while (!(UCSR0A & (1 << UDRE0)))
        ;
    // Put data into buffer and send data
    UDR0 = data;
}

void
UART_putstring(char *StringPtr) {
    while (*StringPtr != 0x00) {
        UART_send(*StringPtr);
        StringPtr++;
    }
}
