/*
 * atmega328pb_inputCompare.c
 *
 * Created: 2/12/2024 4:29:04 PM
 * Author : nmcgill
 */

#include "uart.h"
#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdlib.h>

#define F_CPU                16000000UL   // 16MHz clock
#define USART_BAUD_RATE      9600
#define USART_BAUD_PRESCALER (((F_CPU / (USART_BAUD_RATE * 16UL))) - 1)
#define TIMER_PRESCALER      8
#define __PRINT_NEW_LINE__   UART_putstring(terminalNewLine);

int edge1 = 0;
int edge2 = 0;
char terminalNewLine[] = "\r\n";

void
Initialize() {
    // Disable global interrupts
    cli();

    // Set PB0 (ICP1 pin) to be input
    DDRB &= ~(1 << DDB0);

    // Timer1 setup
    // Set Timer 1 clock to be internally divided by 8
    // 2MHz timer clock, 1 tick = (1/2M) second
    TCCR1B &= ~(1 << CS10);
    TCCR1B |= (1 << CS11);
    TCCR1B &= ~(1 << CS12);

    // Set Timer 1 to Normal
    TCCR1A &= ~(1 << WGM10);
    TCCR1A &= ~(1 << WGM11);
    TCCR1B &= ~(1 << WGM12);
    TCCR1B &= ~(1 << WGM13);

    // Looking for rising edge
    TCCR1B |= (1 << ICES1);

    // Clear input capture flag
    TIFR1 |= (1 << ICF1);

    // Enable global interrupts
    sei();
}

int
main(void) {
    Initialize();   // Set up timer 1 for input captures

    // Set up serial UART printing
    UART_init(USART_BAUD_PRESCALER);
    UART_putstring("ATmega328PB - Input Compare Example");
    __PRINT_NEW_LINE__

    // Measure period between input captures
    while (1) {
        while (!(TIFR1 & (1 << ICF1)))
            ;                   // Wait until the flag is set to 1
        edge1 = ICR1;           // Save value of this edge
        TIFR1 |= (1 << ICF1);   // Clear input capture flag
        UART_putstring("Edge 1");
        __PRINT_NEW_LINE__

        while (!(TIFR1 & (1 << ICF1)))
            ;                   // Wait for change
        TIFR1 |= (1 << ICF1);   // Clear input capture flag
        edge2 = ICR1;
        UART_putstring("Edge 2");
        __PRINT_NEW_LINE__

        int period = F_CPU / TIMER_PRESCALER / (edge2 - edge1);
        char intStringBuffer[10];            // Buffer to hold the converted number
        itoa(period, intStringBuffer, 10);   // Convert integer to string
        UART_putstring("Period:\t");
        UART_putstring(intStringBuffer);
        __PRINT_NEW_LINE__
        __PRINT_NEW_LINE__   // Make space between prints
    }
}