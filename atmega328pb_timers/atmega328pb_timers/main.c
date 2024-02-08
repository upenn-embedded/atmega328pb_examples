#include <avr/io.h>
#include <avr/interrupt.h>


void Initialize()
{
    cli(); // Disable global interrupts

    // Pre-scale 16MHz system clock to 4MHz
    CLKPR = (1 << CLKPCE); // Enable changes to CLKPS bits
                           // Q1: What happens to the output if I don't enable changes?
    CLKPR = (1 << CLKPS1); // Divide by 4
                           // Q2: How might we set it to divide by 8?

    // GPIO pin setup
    DDRB |= (1 << DDB5);    // Set PB5 to be output pin
    PORTB |= (1 << PORTB5); // Start with LED on

    // Timer0 setup
    // Set Timer 0 clock to be internal, div by 8
    TCCR0B &= ~(1 << CS00);
    TCCR0B |= (1 << CS01);
    TCCR0B &= ~(1 << CS02);

    // Set Timer 0 to Clear The Counter (CTC) Mode
    TCCR0A &= ~(1 << WGM00);
    TCCR0A |= (1 << WGM01);
    TCCR0B &= ~(1 << WGM02);
    OCR0A = 249; // (16MHz/4)/(2*8*1+249) = 1kHz

    // Enable interrupt
    TIMSK0 |= (1 << OCIE0A);

    sei(); // Enable global interrupts
}

ISR(TIMER0_COMPA_vect)
{
    PORTB ^= (1 << PORTB5); // Toggle LED

	
}

int main(void)
{
    Initialize();
    while (1);
}
