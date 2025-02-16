#define LED_TOGGLE_EXAMPLE
//#define PULSE_GENERATION_EXAMPLE
//#define FREQUENCY_MEASURE_EXAMPLE

#ifdef LED_TOGGLE_EXAMPLE
// Description: Use Output Compare match to generate a 1Hz square wave with 50% duty cycle
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

#include <xc.h>
#include "../common_libraries/uart.h"
#include <stdio.h> // For sprintf
#include <avr/interrupt.h>

#define F_CPU 16000000UL

int high_time = 999; // 0.5ms * (16MHz/8) - 1 = 999 ticks
int low_time = 3999; // 2ms * (16MHz/8) - 1 = 3999 ticks
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

    // Leave Timer 1 in default, Normal mode

    // Enable Output Compare B interrupt
    TIMSK1 |= (1 << OCIE1B);

    // Toggle OC1B on compare match
    TCCR1A |= (1 << COM1B0);

    // Set initial compare match to kick things off
    // Starting with low signal
    OCR1B = low_time;
    nextSignalLevel = LOGIC_HIGH;

    // Clear interrupt flag
    TIFR1 |= (1 << OCF1B);

    sei(); // Enable global interrupts
}

ISR(TIMER1_COMPB_vect) {
    if (nextSignalLevel == LOGIC_HIGH) {
        OCR1B += high_time;
        nextSignalLevel = LOGIC_LOW;
    } else {
        OCR1B += low_time;
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

#include <xc.h>
#include "../common_libraries/uart.h"
#include <avr/interrupt.h>
#include <stdio.h> // For sprintf

#define F_CPU               16000000UL
#define UART_BAUD_RATE      9600
#define UART_BAUD_PRESCALER (((F_CPU / (UART_BAUD_RATE * 16UL))) - 1)
#define __PRINT_NEW_LINE__  UART_putstring(terminalNewLine);

char terminalNewLine[] = "\r\n";
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

    // Set Timer 1 to Normal
    TCCR1A &= ~(1 << WGM10);
    TCCR1A &= ~(1 << WGM11);
    TCCR1B &= ~(1 << WGM12);
    TCCR1B &= ~(1 << WGM13);

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

    // Timer0, Fast PWM mode
    TCCR0A |= (1 << WGM00);
    TCCR0A |= (1 << WGM01);
    TCCR0B |= (1 << WGM02);

    OCR0A = 39; // Sets frequency, ~390Hz
    OCR0B = OCR0A * 1 / 4; // Sets duty cycle, 75%
    TCCR0A |= (1 << COM0B1); // Non-inverting mode, Clear on Compare Match

    sei(); // Enable global interrupts
}

ISR(TIMER1_CAPT_vect) {
    rising_edge_count++;
}

ISR(TIMER1_COMPA_vect) {
    signal_frequency = rising_edge_count;
    rising_edge_count = 0;
    print_flag = 1; // Let's print the frequency in the main loop
}

int main(void) {
    Initialize();

    UART_init(UART_BAUD_PRESCALER); // Set up serial UART printing
    __PRINT_NEW_LINE__
    UART_putstring("ATmega328PB - Output Compare Frequency Measurement");

    while (1) {
        if (print_flag) {
            print_flag = 0; // Reset the flag
            char intStringBuffer[20]; // Buffer to hold the converted number
            sprintf(intStringBuffer, "Signal Frequency:\t %d Hz", signal_frequency); // Convert integer to string
            __PRINT_NEW_LINE__ // Make space between prints
            UART_putstring(intStringBuffer);
        }
    }
}
#endif