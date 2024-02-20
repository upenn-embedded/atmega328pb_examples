/*
 * atmega328pb_adc.c
 *
 * Created: 2/19/2024 8:23:03 PM
 * Author : nmcgill
 */

#include "../../common_libraries/uart.h"
#include <avr/delay.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdlib.h>

#define F_CPU               16000000UL
#define UART_BAUD_RATE      74880
#define UART_BAUD_PRESCALER (((F_CPU / (UART_BAUD_RATE * 16UL))) - 1)
#define __PRINT_NEW_LINE__  UART_putstring("\r\n");

// Choose ONE example to run (and comment out all others)
// #define ADC_READ_EXAMPLE
// #define TEMPERATURE_READ_EXAMPLE //Still needs a little work!
#define SIGNAL_FREQ_MEASURE_EXAMPLE

#ifdef ADC_READ_EXAMPLE
volatile int hi_or_low = 0;
volatile int count = 0;

void Initialize() {
    // Setup for ADC (10bit = 0-1023)
    // Clear power reduction bit for ADC
    PRR0 &= ~(1 << PRADC);

    // Select Vref = AVcc
    ADMUX |= (1 << REFS0);
    ADMUX &= ~(1 << REFS1);

    // Set the ADC clock div by 128
    // 16M/128=125kHz
    ADCSRA |= (1 << ADPS0);
    ADCSRA |= (1 << ADPS1);
    ADCSRA |= (1 << ADPS2);

    // Select Channel ADC0 (pin C0)
    ADMUX &= ~(1 << MUX0);
    ADMUX &= ~(1 << MUX1);
    ADMUX &= ~(1 << MUX2);
    ADMUX &= ~(1 << MUX3);

    ADCSRA |= (1 << ADATE);   // Autotriggering of ADC

    // Free running mode ADTS[2:0] = 000
    ADCSRB &= ~(1 << ADTS0);
    ADCSRB &= ~(1 << ADTS1);
    ADCSRB &= ~(1 << ADTS2);

    // Disable digital input buffer on ADC pin
    DIDR0 |= (1 << ADC0D);

    // Enable ADC
    ADCSRA |= (1 << ADEN);

    // Start conversion
    ADCSRA |= (1 << ADSC);
}

int main(void) {
    Initialize();
    UART_init(UART_BAUD_PRESCALER);
    while (1) {
        char intStringBuffer[10];         // Buffer to hold the converted number
        itoa(ADC, intStringBuffer, 10);   // Convert integer to string
        UART_putstring("ADC Read:\t");
        UART_putstring(intStringBuffer);
        __PRINT_NEW_LINE__ __PRINT_NEW_LINE__   // Make space between prints
            _delay_ms(5000);
    }
}
#endif

#ifdef TEMPERATURE_READ_EXAMPLE
volatile int hi_or_low = 0;
volatile int count = 0;

void Initialize() {

    cli();   // Disable interrupts

    // Setup for ADC (10bit = 0-1023)
    // Clear power reduction bit for ADC
    PRR0 &= ~(1 << PRADC);

    // Select Vref = Internal 1.1V reference
    ADMUX |= (1 << REFS0);
    ADMUX |= (1 << REFS1);

    // Set the ADC clock div by 128
    // 16M/128=125kHz
    ADCSRA |= (1 << ADPS0);
    ADCSRA |= (1 << ADPS1);
    ADCSRA |= (1 << ADPS2);

    // Select IC Temperature
    ADMUX &= ~(1 << MUX0);
    ADMUX &= ~(1 << MUX1);
    ADMUX &= ~(1 << MUX2);
    ADMUX |= (1 << MUX3);

    ADCSRA |= (1 << ADATE);   // Autotriggering of ADC

    // Free running mode ADTS[2:0] = 000
    ADCSRB &= ~(1 << ADTS0);
    ADCSRB &= ~(1 << ADTS1);
    ADCSRB &= ~(1 << ADTS2);

    // Disable digital input buffer on ADC pin
    DIDR0 |= (1 << ADC0D);

    // Enable ADC
    ADCSRA |= (1 << ADEN);

    // Start conversion
    ADCSRA |= (1 << ADSC);
}

int main(void) {
    Initialize();
    UART_init(UART_BAUD_PRESCALER);
    while (1) {

        int currentAdcRead = ADC;
        // Datasheet 28.8: Temperature conversion
        // T = { [(ADCH << 8) | ADCL] - TOS} / k
        char rawAdcStringBuffer[10];                    // Buffer to hold the converted number
        itoa(currentAdcRead, rawAdcStringBuffer, 10);   // Convert integer to string
        UART_putstring("Raw ADC Read:\t");
        UART_putstring(rawAdcStringBuffer);
        // UART_putstring("IC Temperature:\t");
        // UART_putstring(intStringBuffer);
        __PRINT_NEW_LINE__ __PRINT_NEW_LINE__   // Make space between prints
            _delay_ms(5000);
    }
}
#endif

#ifdef SIGNAL_FREQ_MEASURE_EXAMPLE

volatile int hi_or_low = 0;
volatile int count = 0;
volatile int print_flag = 0;

void Initialize() {
    cli();   // Disable global interrupts

    // Setup for ADC (10bit = 0-1023)
    // Clear power reduction bit for ADC
    PRR0 &= ~(1 << PRADC);

    // Select Vref = AVcc
    ADMUX |= (1 << REFS0);
    ADMUX &= ~(1 << REFS1);

    // Set the ADC clock div by 128
    // 16M/128=125kHz
    ADCSRA |= (1 << ADPS0);
    ADCSRA |= (1 << ADPS1);
    ADCSRA |= (1 << ADPS2);

    // Select Channel ADC0 (pin C0)
    ADMUX &= ~(1 << MUX0);
    ADMUX &= ~(1 << MUX1);
    ADMUX &= ~(1 << MUX2);
    ADMUX &= ~(1 << MUX3);

    ADCSRA |= (1 << ADATE);   // Autotriggering of ADC

    // Free running mode ADTS[2:0] = 000
    ADCSRB &= ~(1 << ADTS0);
    ADCSRB &= ~(1 << ADTS1);
    ADCSRB &= ~(1 << ADTS2);

    DIDR0 |= (1 << ADC0D);   // Disable digital input buffer on ADC pin
    ADCSRA |= (1 << ADEN);   // Enable ADC
    ADCSRA |= (1 << ADSC);   // Start conversion

    // Timer 1 Setup
    // Timer 1 Clock divided by 256
    // 62.5kHz timer clock, 1 tick = (1/62.5k) second
    // Can read frequencies from ~1Hz to ~62.5kHz
    TCCR1B |= (1 << CS12);
    // CTC Mode
    TCCR1B |= (1 << WGM12);
    // Compare Match A Interrupt
    TIMSK1 |= (1 << OCIE1A);
    // Clear the interrupt flag
    TIFR1 |= (1 << OCF1A);
    OCR1A = 62500;   // 1 Second

    sei();   // Enable global interrupts
}

ISR(ADC_vect) {
    // Count the low points
    if (ADC < 5 && !hi_or_low) {
        count++;
        hi_or_low = 1;
    }
    if (ADC > 1000) {
        hi_or_low = 0;
    }
}

ISR(TIMER1_COMPA_vect) { print_flag = 1; }

int main(void) {
    Initialize();
    UART_init(UART_BAUD_PRESCALER);
    while (1) {
        if (print_flag) {
            print_flag = 0;                     // Clear the print flag
            char intStringBuffer[10];           // Buffer to hold the converted number
            itoa(count, intStringBuffer, 10);   // Convert integer to string
            UART_putstring("Count:\t");
            UART_putstring(intStringBuffer);
            __PRINT_NEW_LINE__   // Make space between prints
        }
    }
}
#endif