#include <avr/io.h>
#include <avr/interrupt.h> // Q: What happens if I don't include this?

void Initialize()
{
    DDRB |= (1 << DDB5);	 // Set PB5 as an output
    PORTB |= (1 << PORTB5);   // Turn off the User LED by default
	
    DDRB &= ~(1 << DDB7);	 // Set PB7 as input
    //PORTB |= (1 << PORTB7);  // Enable pull-up resistor on PB7

    PCICR |= (1 << PCIE0);	 // Enable Pin Change Interrupt for PCINT0-7
    PCMSK0 |= (1 << PCINT7); // Enable Pin Change Interrupt for PB7
    sei();					 // Enable global interrupts
}

ISR(PCINT0_vect) {
		PORTB ^= (1 << PORTB5);	// Toggle the LED
}

int main(void)
{
	Initialize();
    while (1) 
    {
    }
}

