// #define LED_TOGGLE_EXAMPLE
//#define PULSE_GENERATION_EXAMPLE
 #define FREQUENCY_MEASURE_EXAMPLE

#ifdef LED_TOGGLE_EXAMPLE
// Description: Use Output Compare match to generate a 1Hz square wave with 50% duty cycle
#define F_CPU 16000000UL
#include <xc.h>
#include "../common_libraries/uart.h"

void Initialize() {

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

}

int main(void) {
    Initialize();
    while (1);
}
#endif



#ifdef PULSE_GENERATION_EXAMPLE
// Description: Use Output Compare to generate a pulse, high for 0.5ms and low for 2ms

#define F_CPU 16000000UL
#include <xc.h>
#include <avr/interrupt.h>

int high_time_ticks = 1000; // 0.5ms * (16MHz/8) = 1000 ticks
int low_time_ticks = 4000; // 2ms * (16MHz/8) = 4000 ticks
#define LOGIC_HIGH 1
#define LOGIC_LOW 0
volatile int nextSignalLevel; // high=1, low=0

void Initialize() {

    cli(); // Disable global interrupts

    // Set PB2 (OC1B pin) to be an output pin
    DDRB |= (1 << DDB2);

    // Timer setup
    // Set Timer 1 clock to be internally divided by 8
    // System clock is 16MHz, Timer1 clock is 16M/8 = 2MHz
    TCCR1B |= (1 << CS11);

    // Leave Timer 1 in its default, Normal mode (Mode 0)
    // The Waveform Generation Mode bits are all zero by default
    // The timer will count up to 0xFFFF and then overflow back to 0x0000

    // Enable Output Compare B Match Interrupt
    TIMSK1 |= (1 << OCIE1B);

    // Toggle OC1B (pin PB2) on compare match
    TCCR1A |= (1 << COM1B0);

    // Set initial compare match to kick things off
    // Subtract one to account for starting at 0
    // Starting with low signal
    OCR1B = low_time_ticks-1;
    nextSignalLevel = LOGIC_HIGH;

    // Clear interrupt flag
    TIFR1 |= (1 << OCF1B);

    sei(); // Enable global interrupts
}

ISR(TIMER1_COMPB_vect) {
    // This ISR triggers every time the OCR1B value matches the Counter (TCNT1)
    // TCNT1 counts from 0 to 0xFFFF, then overflows back to 0x0000.
    // TCNT1 does NOT reset to 0 on compare match! It keeps counting.
    // We need to adjust the OCR1B value at every compare match to set the correct pulse width.
    
    // What happens on overflow? It works! We just need the low and high pulse times to be correct.
    // Example: When TCNT = OCR1B = 65000, the signal toggles to LOW. What should we set OCR1B to?
    // Add the LOW time (4000 ticks) to the existing OCR1B value to set the next transition time.
    // OCR1B = 65000 + 4000 = 69000, but because this is a 16-bit int, we can't fit that!
    // It will overflow to 69000-65536 = 3464.
    // The signal will turn HIGH at TCNT1 = 3464.

    if (nextSignalLevel == LOGIC_HIGH) {
        OCR1B += high_time_ticks;
        nextSignalLevel = LOGIC_LOW;
    } else {
        OCR1B += low_time_ticks;
        nextSignalLevel = LOGIC_HIGH;
    }
}

int main(void) {
    Initialize();
    while (1);
}
#endif

#ifdef FREQUENCY_MEASURE_EXAMPLE
// Description: Measure a signal using output compare as the baseline and input capture as the counter

#define F_CPU               16000000UL
#include <xc.h>
#include "../common_libraries/uart.h"
#include <avr/interrupt.h>
#include <stdio.h> // For sprintf

#define UART_BAUD_RATE      74880
#define UART_BAUD_PRESCALER (((F_CPU / (UART_BAUD_RATE * 16UL))) - 1)

volatile int print_flag = 0;
volatile int rising_edge_count = 0;
volatile int signal_frequency = 0;

void Initialize() {

    cli(); // Disable global interrupts

    DDRB &= ~(1 << DDB0); // Set PB0 (ICP1 pin) to be an input

    // Timer setup
    // Set Timer 1 clock to be internally divided by 256
    // System clock is 16MHz, Timer1 clock is 16M/256 = 62.5kHz
    // Can read frequencies from ~1Hz to ~62.5kHz
    TCCR1B |= (1 << CS12);

    // Leave Timer 1 in its default, Normal mode (Mode 0)
    // The Waveform Generation Mode bits are all zero by default
    // The timer will count up to 0xFFFF and then overflow back to 0x0000

    TCCR1B |= (1 << ICES1); // Looking for rising edge
    TIMSK1 |= (1 << ICIE1); // Enable Input Capture interrupt
    TIMSK1 |= (1 << OCIE1A); // Enable Output Compare interrupt
    OCR1A = 62499; // 1s benchmark
    TIFR1 |= (1 << ICF1); // Clear interrupt flag

    // ==== Sample waveform setup
    DDRD |= (1 << DDD5); // Set PD5 as OC0B

    // Timer0, prescale (/1024)
    TCCR0B |= (1 << CS00);
    TCCR0B |= (1 << CS02);

    // Timer0, Fast PWM (Mode 7)
    TCCR0A |= (1 << WGM00);
    TCCR0A |= (1 << WGM01);
    TCCR0B |= (1 << WGM02);

    OCR0A = 32; // Sets frequency, ~Hz
    OCR0B = OCR0A * 3 / 4; // Sets duty cycle, 75%
    // Set up Output Compare
    // Non-inverting mode, Clear on Compare Match
    TCCR0A |= (1 << COM0B1); 

    sei(); // Enable global interrupts
}

ISR(TIMER1_CAPT_vect) {
    rising_edge_count++;
}

ISR(TIMER1_COMPA_vect) {
    signal_frequency = rising_edge_count;
    print_flag = 1; // Let's print the frequency in the main loop
    rising_edge_count = 0;
}

int main(void) {
    Initialize();

    UART_init(UART_BAUD_PRESCALER); // Set up serial UART printing
    __PRINT_NEW_LINE__
    UART_putstring("ATmega328PB - Output Compare Frequency Measurement");

    char intStringBuffer[10]; // Buffer to hold the converted number
    while (1) {
        if (print_flag) {
            print_flag = 0; // Reset the flag
            sprintf(intStringBuffer, "\r\n%d Hz", signal_frequency); // Convert integer to string
            // __PRINT_NEW_LINE__ // Make space between prints
            UART_putstring(intStringBuffer);
        }
    }
}
#endif