#include <xc.h>
#include "../common_libraries/uart.h"

#define F_CPU                16000000UL   // 16MHz clock
#define USART_BAUD_RATE      9600
#define USART_BAUD_PRESCALER (((F_CPU / (USART_BAUD_RATE * 16UL))) - 1)

int main(void) {
    // Set up serial UART printing
    UART_init(USART_BAUD_PRESCALER);
    __PRINT_NEW_LINE__
    UART_putstring("ATmega328PB - Serial UART Printing Example\r\n");

    // Main loop
    while (1)
        ;
}
