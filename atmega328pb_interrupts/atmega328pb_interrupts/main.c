#include <avr/io.h>
#include <avr/interrupt.h> // Q: What happens if I don't include this?

void Initialize()
{
    DDRB |= (1 << DDB5);	 // Set PB5 as an output
    PORTB |= (1 << PORTB5);   // Turn off the User LED by default
	
    DDRD &= ~(1 << DDD5);	 // Set PD5 as input
    PORTD |= (1 << PORTD7);  // Enable pull-up resistor on PD5

    PCICR |= (1 << PCIE2);	 // Enable Pin Change Interrupt for PCINT16-23
    PCMSK2 |= (1 << PCINT21); // Enable Pin Change Interrupt for PD5
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

