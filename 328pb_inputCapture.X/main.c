// Uncomment only one of these at a time!
#define INTERRUPT_EXAMPLE
//#define POLLING_EXAMPLE 

#ifdef INTERRUPT_EXAMPLE
#include <xc.h>
#include "../common_libraries/uart.h"
#include <avr/interrupt.h>
#include <stdio.h>

#define F_CPU               16000000UL   // 16MHz clock
#define UART_BAUD_RATE      9600
#define UART_BAUD_PRESCALER (((F_CPU / (UART_BAUD_RATE * 16UL))) - 1)
#define TIMER_PRESCALER     8
#define __PRINT_NEW_LINE__  UART_putstring(terminalNewLine);

char terminalNewLine[] = "\r\n";

volatile int edge = 0;
volatile int period = 0;
volatile int print_flag = 0;

void Initialize() {

    cli(); // Disable global interrupts

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


    // Enable input capture interrupt
    TIMSK1 |= (1 << ICIE1);


    sei(); // Enable global interrupts
}

ISR(TIMER1_CAPT_vect) {
    period = ICR1 - edge;
    edge = ICR1;
    print_flag = 1;
}

int main(void) {
    Initialize(); // Set up timer 1 for input captures

    // Set up serial UART printing
    UART_init(UART_BAUD_PRESCALER);
    UART_putstring("ATmega328PB - Input Capture Period Measurement");
    __PRINT_NEW_LINE__

    while (1) {

        if (print_flag) {
            print_flag = 0;
            char intStringBuffer[20]; // Buffer to hold the converted number
            sprintf(intStringBuffer, "Period:\t %d", period); // Convert integer to string
            UART_putstring(intStringBuffer);
            __PRINT_NEW_LINE__
            __PRINT_NEW_LINE__ // Make space between prints
        }
    }
}
#endif



// Polling Example
#ifdef POLLING_EXAMPLE
#include <xc.h>
#include "../common_libraries/uart.h"
#include <avr/interrupt.h>
#include <stdio.h>


#define F_CPU               16000000UL   // 16MHz clock
#define UART_BAUD_RATE      9600
#define UART_BAUD_PRESCALER (((F_CPU / (UART_BAUD_RATE * 16UL))) - 1)
#define TIMER_PRESCALER     8
#define __PRINT_NEW_LINE__  UART_putstring(terminalNewLine);

char terminalNewLine[] = "\r\n";

int period = 0;
int edge1 = 0;
int edge2 = 0;

void Initialize() {

    cli(); // Disable global interrupts

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

    sei(); // Enable global interrupts
}

int main(void) {
    Initialize(); // Set up timer 1 for input captures

    // Set up serial UART printing
    UART_init(UART_BAUD_PRESCALER);
    UART_putstring("ATmega328PB - Input Capture Period Measurement");
    __PRINT_NEW_LINE__

    while (1) {
        // Rising Edge 1 Capture
        while (!(TIFR1 & (1 << ICF1)))
            ; // Wait until the flag is set to 1
        edge1 = ICR1; // Save value of this edge
        TIFR1 |= (1 << ICF1); // Clear input capture flag

        // Rising Edge 2 Capture
        while (!(TIFR1 & (1 << ICF1)))
            ; // Wait for change
        TIFR1 |= (1 << ICF1); // Clear input capture flag
        edge2 = ICR1;

        // Calculate period
        period = F_CPU / TIMER_PRESCALER / (edge2 - edge1);

        char intStringBuffer[20]; // Buffer to hold the converted number
        sprintf(intStringBuffer, "Period:\t %d", period); // Convert integer to string
        UART_putstring(intStringBuffer);
        __PRINT_NEW_LINE__
        __PRINT_NEW_LINE__ // Make space between prints
    }
}
#endif