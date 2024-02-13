/*
 * atmega328pb_pwm.c
 *
 * Created: 2/13/2024 12:01:21 AM
 * Author : nmcgill
 */

#include <avr/interrupt.h>
#include <avr/io.h>

int high_time = 10907;
int low_time = 25451;
volatile int high_low = 1;   // start high

void Initialize() {

    cli();   // Disable global interrupts

    DDRB |= (1 << DDB1);      // Set PB1 to be an output pin
    PORTB |= (1 << PORTB1);   // Start with output on

    // Timer1, prescale of 1
    TCCR1B |= (1 << CS10);

    // Timer1, CTC mode
    TCCR1B |= (1 << WGM12);

    // Toggle on compare match
    TCCR1A |= (1 << COM1A0);
    OCR1A = 20;

    TIMSK1 |= (1 << OCIE1A);   // Enable Interrupt

    TIFR1 |= (1 << OCF1A);   // Clear interrupt flag

    sei();   // Enable global interrupts
}

ISR(TIMER1_COMPA_vect) {
    if (high_low) {
        OCR1A = low_time;
        high_low = 0;
    } else {
        OCR1A = high_time;
        high_low = 1;
    }
}

int main(void) {
    Initialize();
    while (1)
        ;
}
