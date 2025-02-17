// -------------------------------------
// Uncomment only example at a time!
//#define CTC_PWM_EXAMPLE
//#define FAST_PWM_EXAMPLE
//#define PHASE_CORRECT_VS_FAST_EXAMPLE
// -------------------------------------

#ifdef CTC_PWM_EXAMPLE
// Description: Generate a 400Hz PWM signal with a 30% duty cycle
// using CTC mode with interrupts on Timer 1
// Calculation: 440Hz @ 30% duty cycle
// Period = 1/440Hz = ~2ms
// High time = 30% * 2ms = 0.6ms
// Low time = 70% * 2ms = 1.4ms
// For 440Hz 50% duty cycle: Count = (16MHz/(2*1*440Hz))-1 = 18180
// Total count in period = 2*18180 = 36360
// Adjusting for 30% duty cycle: (36360 * 0.3)-1 = 10907
// Adjusting for 70% duty cycle: (36360 * 0.7)-1 = 25451
// Subtracting 1 because the count starts at 0

#include <xc.h>
#include <avr/interrupt.h>

int high_time = 10907;
int low_time = 25451;
#define LOGIC_HIGH 1
#define LOGIC_LOW 0
volatile int nextSignalLevel; // high=1, low=0

void Initialize() {

    cli(); // Disable global interrupts

    DDRB |= (1 << DDB1); // Set PB1 to be an output pin
    PORTB |= (1 << PORTB1); // Start with output on

    TCCR1B |= (1 << CS10); // Timer1, prescale of 1
    TCCR1B |= (1 << WGM12); // Timer1, CTC mode
    TCCR1A |= (1 << COM1A0); // Toggle on compare match
    OCR1A = low_time;
    nextSignalLevel = LOGIC_LOW; // Starting with low signal

    TIMSK1 |= (1 << OCIE1A); // Enable Interrupt
    TIFR1 |= (1 << OCF1A); // Clear interrupt flag

    sei(); // Enable global interrupts
}

ISR(TIMER1_COMPA_vect) {
    if (nextSignalLevel == LOGIC_HIGH) {
        OCR1A = low_time;
        nextSignalLevel = LOGIC_LOW;
    } else {
        OCR1A = high_time;
        nextSignalLevel = LOGIC_HIGH;
    }
}

int main(void) {
    Initialize();
    while (1);
}
#endif


#ifdef FAST_PWM_EXAMPLE
// Description: Generate a 400kHz PWM signal with a 75% duty cycle
// using Fast PWM (no interrupts!) on Timer 0
// Calculation: 440kHz @ 75% duty cycle
// TOP = OCR0A = (16MHz/(1*440kHz))-1 = 39

#include <xc.h>

#define F_CPU 16000000UL

void Initialize() {
    DDRD |= (1 << DDD5); // Set PD5 as OC0B

    TCCR0B |= (1 << CS00); // Timer0, prescale of 1

    // Timer0, Fast PWM mode
    TCCR0A |= (1 << WGM00);
    TCCR0A |= (1 << WGM01);
    TCCR0B |= (1 << WGM02);

    OCR0A = 39; // Sets frequency, 400kHz
    OCR0B = OCR0A * 1 / 4; // Sets duty cycle

    // Non-inverting mode, Clear on Compare Match
    TCCR0A |= (1 << COM0B1);
}

int main(void) {
    Initialize();
    while (1);
}
#endif


#ifdef PHASE_CORRECT_VS_FAST_EXAMPLE
#include <xc.h>
#define F_CPU 16000000UL

void Initialize() {
    // === Set up Fast PWM mode (Mode 3) on Timer 0
    // Set PD5 (OC0B) and PD6 (OC0A) as outputs
    DDRD |= (1 << DDD5) | (1 << DDD6);

    // Timer0, prescale of 64
    TCCR0B |= (1 << CS01) | (1 << CS00);

    // Timer0, Fast PWM mode (Mode 3)
    TCCR0A |= (1 << WGM00) | (1 << WGM01);
    TCCR0B &= ~(1 << WGM02);

    // Clear on Compare Match, Non-inverting mode
    TCCR0A |= (1 << COM0A1) | (1 << COM0B1);
    TCCR0A &= ~((1 << COM0A0) | (1 << COM0B0));

    OCR0A = 200; // Sets frequency
    OCR0B = OCR0A * 1 / 4; // Sets duty cycle, 75%

    // === Set up Phase Correct PWM mode (Mode 1) on Timer 1
    // Set PB1 (OC1A) and PB2 (OC1B) as outputs
    DDRB |= (1 << DDB1) | (1 << DDB2);

    // Timer1, prescale of 64
    TCCR1B |= (1 << CS11) | (1 << CS10);

    // Timer1, Phase Correct PWM mode, 8-bit (Mode 1)
    TCCR1A |= (1 << WGM10);

    // Clear on Compare Match, Non-inverting mode
    TCCR1A |= (1 << COM1A1) | (1 << COM1B1);
    TCCR1A &= ~((1 << COM1A0) | (1 << COM1B0));

    OCR1A = 200; // Sets frequency
    OCR1B = OCR1A * 1 / 4; // Sets duty cycle, 75%
}

int main(void) {
    Initialize();
    while (1);
}
#endif