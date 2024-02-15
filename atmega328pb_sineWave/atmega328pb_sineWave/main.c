/*
 * atmega328pb_sineWave.c
 *
 * Created: 2/14/2024 6:36:25 PM
 * Author : nmcgill
 */

#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdlib.h>

#define F_CPU 16000000UL

int size = 100;

const int sine_table_const[100] = {128, 136, 143, 151, 159, 167, 174, 182, 189, 196, 202, 209, 215, 220, 226, 231, 235, 239, 243, 246, 249, 251, 253, 254, 255,
                                   255, 255, 254, 253, 251, 249, 246, 243, 239, 235, 231, 226, 220, 215, 209, 202, 196, 189, 182, 174, 167, 159, 151, 143, 136,
                                   128, 119, 112, 104, 96,  88,  81,  73,  66,  59,  53,  46,  40,  35,  29,  24,  20,  16,  12,  9,   6,   4,   2,   1,   0,
                                   0,   0,   1,   2,   4,   6,   9,   12,  16,  20,  24,  29,  35,  40,  46,  53,  59,  66,  73,  81,  88,  96,  104, 112, 119};

volatile int lookup_count = 0;

void Initialize(void) {
    // Disable global interrupts
    cli();

    // GPIO pin setup
    DDRB |= (1 << DDB1);   // OC1A - PB1
    DDRB |= (1 << DDB5);   // Debug output pin

    // Timer 1 Setup
    // Prescale of 1
    TCCR1B |= (1 << CS10);

    // Set Phase Correct PWM
    // 8 bit, count to TOP
    TCCR1A |= (1 << WGM10);

    // OCR1A sets duty cycle
    OCR1A = 5;

    // Enable Timer Overflow Interrupt
    TIMSK1 |= (1 << TOIE1);
    TIFR1 |= (1 << TOV1);

    // Non-inverting mode
    // Clear on compare match
    TCCR1A |= (1 << COM1A1);

    // Enable global interrupts
    sei();
}

ISR(TIMER1_OVF_vect) {
    // Toggle this pin every time OCR1A is updated
    PORTB ^= (1 << PORTB5);
    if (lookup_count < size) {
        OCR1A = sine_table_const[lookup_count];
        lookup_count++;
    } else {
        lookup_count = 0;
    }
}

int main(void) {
    Initialize();
    while (1)
        ;
}
