#include <xc.h>
// MPLAB uses a slightly different compiler by default, the XC8
// <xc.h> is preferred to <avr/io.h>

void Initialize()
{
    DDRB |= (1 << DDB5);	 // Set PB5 as an output
    PORTB |= (1 << PORTB5);   // Turn off the User LED by default
	
    DDRB &= ~(1 << DDB7);	 // Set PB7 as input, Active LOW switch
    PORTB |= (1 << PORTB7);  // Enable pull-up resistor on PD5
}

int main(void)
{
	Initialize();
    while (1) 
    {
        if(PINB & (1<<PINB7))
        {
            PORTB &= ~(1 << PORTB5); // Turn OFF the LED
        }   
        else
        {
            PORTB |= (1 << PORTB5);	// Turn ON the LED
        }   
    }
}

