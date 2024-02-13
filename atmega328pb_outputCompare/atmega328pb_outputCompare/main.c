/*
 * atmega328pb_outputCompare.c
 *
 * Created: 2/12/2024 11:19:36 PM
 * Author : nmcgill
 */

// #define LED_TOGGLE_EXAMPLE
// #define PULSE_GENERATION_EXAMPLE
#define FREQUENCY_MEASURE_EXAMPLE

#ifdef LED_TOGGLE_EXAMPLE
#include <avr/interrupt.h>
#include <avr/io.h>

void Initialize() {

    cli();   // Disable global interrupts

    // Set PB1 (OC1A pin) to be an output pin
    DDRB |= (1 << DDB1);

    // Timer setup
    // Set Timer 1 clock to be internally divided by 256
    // System clock is 16MHz, Timer1 clock is 16M/256 = 62.5kHz
    TCCR1B |= (1 << CS12);

    // Set Timer 1 to CTC (Clear Timer on Compare match)
    TCCR1B |= (1 << WGM12);

    // Toggle OC1A on compare match
    TCCR1A |= (1 << COM1A0);

    // 1Hz square wave with 50% duty cycle
    // OCRnx (Count) = (16MHz/(2*256 prescaler * 1Hz)) - 1 = 31249
    OCR1A = 31249;

    sei();   // Enable global interrupts
}

int main(void) {
    Initialize();
    while (1)
        ;
}
#endif

#ifdef PULSE_GENERATION_EXAMPLE
#include <avr/interrupt.h>
#include <avr/io.h>

#define F_CPU 16000000UL

int high_time = 999;            // 0.5ms * (16MHz/8) - 1 = 999 ticks
int low_time = 3999;            // 2ms * (16MHz/8) - 1 = 3999 ticks
volatile int high_or_low = 1;   // high=1, low=0

void Initialize() {

    cli();   // Disable global interrupts

    // Set PB2 (OC1B pin) to be an output pin
    DDRB |= (1 << DDB2);

    // Timer setup
    // Set Timer 1 clock to be internally divided by 8
    // System clock is 16MHz, Timer1 clock is 16M/8 = 2MHz
    TCCR1B |= (1 << CS11);

    // Leave Timer 1 in default, Normal mode

    // Enable Output Compare B interrupt
    TIMSK1 |= (1 << OCIE1B);

    // Toggle OC1B on compare match
    TCCR1A |= (1 << COM1B0);
    OCR1B = 100;             // PB2 will be quickly pulled high
    TIFR1 |= (1 << OCF1B);   // Clear interrupt flag

    sei();   // Enable global interrupts
}

ISR(TIMER1_COMPB_vect) {
    if (high_or_low) {
        OCR1B += high_time;
        high_or_low = 0;
    } else {
        OCR1B += low_time;
        high_or_low = 1;
    }
}

int main(void) {
    Initialize();
    while (1)
        ;
}
#endif

#ifdef FREQUENCY_MEASURE_EXAMPLE
#include "../../common_libraries/uart.h"
#include <avr/interrupt.h>
#include <avr/io.h>

#define F_CPU               16000000UL
#define UART_BAUD_RATE      9600
#define UART_BAUD_PRESCALER (((F_CPU / (UART_BAUD_RATE * 16UL))) - 1)
#define __PRINT_NEW_LINE__  UART_putstring(terminalNewLine);

char terminalNewLine[] = "\r\n";
volatile int print_flag = 0;

volatile int count = 0;
volatile int freq = 0;

void Initialize() {

    cli();   // Disable global interrupts

    DDRB &= ~(1 << DDB0);   // Set PB0 (ICP1 pin) to be an input

    // Timer setup
    // Set Timer 1 clock to be internally divided by 256
    // System clock is 16MHz, Timer1 clock is 16M/256 = 62.5kHz
    // Can read frequencies from ~1Hz to ~62.5kHz
    TCCR1B |= (1 << CS12);

    // Set Timer 1 to Normal
    TCCR1A &= ~(1 << WGM10);
    TCCR1A &= ~(1 << WGM11);
    TCCR1B &= ~(1 << WGM12);
    TCCR1B &= ~(1 << WGM13);

    TCCR1B |= (1 << ICES1);   // Looking for rising edge

    TIMSK1 |= (1 << ICIE1);   // Enable input capture interrupt

    TIMSK1 |= (1 << OCIE1A);   // Enable Output Compare interrupt
    OCR1A = 62499;             // 1s

    TIFR1 |= (1 << ICF1);   // Clear interrupt flag

    sei();   // Enable global interrupts
}

ISR(TIMER1_CAPT_vect) { count++; }

ISR(TIMER1_COMPA_vect) {
    freq = count;
    count = 0;
    print_flag = 1;
}

int main(void) {
    Initialize();

    // Set up serial UART printing
    UART_init(UART_BAUD_PRESCALER);
    UART_putstring("ATmega328PB - Output Compare Frequency Measurement");
    __PRINT_NEW_LINE__

    while (1) {
        if (print_flag) {
            print_flag = 0;
            char intStringBuffer[10];          // Buffer to hold the converted number
            itoa(freq, intStringBuffer, 10);   // Convert integer to string
            UART_putstring("freq: \t");
            UART_putstring(intStringBuffer);
            __PRINT_NEW_LINE__   // Make space between prints
        }
    }
}
#endif