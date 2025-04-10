#include <avr/io.h>
#include <avr/interrupt.h>

int windSpdRaw;
bool windTick = 1; //required as the wind speed sensor is contact based, and the signal can vary in length based on wind speed
float windSpd;


//N.B. Very unoptimized code, just trying to get something that works
int main(void)
{
    TCCR1B |= (1 << WGM12);     // Set for CTC mode.  OCR1A = modulus
    TIMSK1 |= (1 << OCIE1A);    // Enable CTC interrupt
    sei();                      // Enable global interrupts
    OCR1A = 38400;              // Set the counter modulus (for 0.25hz)
    TCCR1B |= ((1 << CS10) | (1 << CS12));      // Set prescaler for divide by 1024,
                                // also starts timer
    


    while (1) {

    }

    return 0;   
}


ISR(TIMER1_COMPA_vect)
{
    windSpd = 0.6 * windSpdRaw; //runs every 4 seconds, 2.4km windspeed @ 1 tick/sec
}

ISR(PCINT1_vect)
{
    if(PINC & windTick)
    {
        windSpdRaw += 1;
        windTick = 0;
    }
    else if (~PINC & ~windTick)
    {
        windTick = 1;
    }
}
        

weather_kit_init(){
    PCICR |= (1 << PCIE1);  // Enable PCINT on Port C
    PCMSK1 |= (1 << PCINT12 | 1 << PCINT11); // Interrupt on PC4, PC3
}



