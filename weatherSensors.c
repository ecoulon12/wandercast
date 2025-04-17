#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdbool.h>
#include "lcd.h"

int windSpdRaw = 0;
bool windTick = true; //required as the wind speed sensor is contact based, and the signal can vary in length based on wind speed
float windSpd = 0;


//N.B. Very unoptimized code, just trying to get something that works
void weatherSensorsSW(void)
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
    char printData[32];
    if(PINC && windTick)
    {
        windSpdRaw += 1;
        windTick = 0;
        snprintf(printData, 32, "Wind Speed =%02x",windSpdRaw);
        lcd_write_string(printData);
    }
    else if (~PINC & ~windTick)
    {
        windTick = 1;
    }
}
        

void weather_kit_init(){
    PCICR |= (1 << PCIE1);  // Enable PCINT on Port C
    PCMSK1 |= (1 << PCINT10 | 1 << PCINT11); // Interrupt on PC2, PC3
}



