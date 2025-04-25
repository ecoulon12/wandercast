#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <util/delay.h>
#include "lcd.h"

void weatherSensors_init();

int windSpdRaw = 0;
bool windTick = true; //required as the wind speed sensor is contact based, and the signal can vary in length based on wind speed
float windSpd = 0;
float windDirRaw;




//N.B. Very unoptimized code, just trying to get something that works
void weatherSensors(void)
{
    //Moved all init code to init function
 
}


ISR(TIMER1_COMPA_vect)
{
    // windSpd = 0.6 * windSpdRaw; //runs every 4 seconds, 2.4km windspeed @ 1 tick/sec
}


ISR(PCINT1_vect)
{
    //code for anenometer, linked to PC2
    lcd_write_string("interrupt");
    _delay_ms(500);
    lcd_clear_screen();
    /*
    char printDataSpd[32];
    if((PINC & (1 << PC2)) && windTick) 
    {
        windSpdRaw += 1;
        windTick = 0;
        snprintf(printDataSpd, 32, "Wind Speed =%02x",windSpdRaw); // print function to test
        lcd_write_string(printData);
    }
    else if (~PINC & ~windTick)
    {
        windTick = 1;
    }
    */
}

void windVane(){ 
    char printVane[20];
    ADCSRA |= (1 << ADSC); // Starts ADC sample cycle
    while((ADCSRA & (1 << ADSC)) != 0){ // Waits for ADC sample to return
        
    }
    windDirRaw = ADCH;

    snprintf(printVane, 20, "ADC=%8x, wDR = %03D", ADCH, windDirRaw);
    lcd_write_string(printVane);
    _delay_ms(1000);
    lcd_clear_screen();
    // Sample retrieved

    /*CALIBRATION VALUES
    (V/5) * 255 = ADC
    DIR|   V   | ADC
    N  = 3.853 = 196.5 
    NE = 2.277 = 116.1
    E  = 0.465 = 23.7
    SE = 0.915 = 46.7
    S  = 1.425 = 72.7
    SW = 3.097 = 158.0
    W  = 4.621 = 235.7
    NW = 4.341 = 221.4 */
    if((windDirRaw <= 200) && (windDirRaw >= 190)){
        lcd_write_string("Wind Dir = N");
    }
    else if((windDirRaw <= 120) && (windDirRaw >= 110)){
        lcd_write_string("Wind Dir = NE");
    }
    else if((windDirRaw <= 30) && (windDirRaw >= 20)){
        lcd_write_string("Wind Dir = E");
    }
    else if((windDirRaw <= 50) && (windDirRaw >= 40)){
        lcd_write_string("Wind Dir = SE");
    }
    else if((windDirRaw <= 80) && (windDirRaw >= 70)){
        lcd_write_string("Wind Dir = S");
    }
    else if((windDirRaw <= 160) && (windDirRaw >= 150)){
        lcd_write_string("Wind Dir = SW");
    }
    else if((windDirRaw <= 240) && (windDirRaw >= 230)){
        lcd_write_string("Wind Dir = W");
    }
    else if((windDirRaw <= 225) && (windDirRaw >= 215)){
        lcd_write_string("Wind Dir = NW");
    }
    else{
        lcd_write_string("Wind Dir = XX");
    }

    _delay_ms(1000);
}

        

void weatherSensors_init(){
    TCCR1B |= (1 << WGM12);     // Set for CTC mode.  OCR1A = modulus
    TIMSK1 |= (1 << OCIE1A);    // Enable CTC interrupt
     sei();                      // Enable global interrupts
    OCR1A = 38400;              // Set the counter modulus (for 0.25hz)
    TCCR1B |= ((1 << CS10) | (1 << CS12));      // Set prescaler for divide by 1024,
                                                // also starts timer
    PCICR |= (1 << PCIE1);  // Enable PCINT on Port C
    PCMSK1 |= (1 << PCINT10 | 1 << PCINT11); // Interrupt on PC2, PC3
    ADMUX |= ((1 << REFS0) | (1 << ADLAR) | (1 << MUX0)); /*  -Sets ADC range from 0-5V (REFS register)
                                                        -Sets readings to 8-bit definition (ADLAR)
                                                        -Sets active read channel to PC1 (MUX register)*/
    ADCSRA |= ((1 << ADPS0) | (1 << ADPS1) | (1 << ADPS2) | (1 << ADEN)); //Sets ADC clock scalar to 128 (ADPS register) and enables ADC (ADEN)
    lcd_write_string("weatherSensor init");
    _delay_ms(500);
}



