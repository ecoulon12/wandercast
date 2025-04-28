#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <util/delay.h>
#include "lcd.h"


//global vars
int windSpdRaw = 0;
bool windFlag = 0;
double windSpd;
double windDirRaw;



ISR(TIMER1_COMPA_vect)
{
    // windSpd = (windSpdRaw / 10);
    // if((windSpdRaw % 5 == 0) && (windSpd != 0)){
    //     windSpd += 2;
    // }
    
    //  //runs every 4 seconds, 2.4km windspeed @ 1 tick/sec
    char printIntrpt[20];
    snprintf(printIntrpt, 20, "Spd = %2d", windSpdRaw / 2);
    lcd_write_string(printIntrpt);
    _delay_ms(100);

    // windSpdRaw = 0;
}


ISR(PCINT1_vect)
{
    
    // lcd_write_string("interrupt");
    // _delay_ms(500);
    // lcd_clear_screen();
    
    if(PINC & (1 << PC2))  //code for anenometer, linked to PC2
    {
        windSpdRaw += 1;
    }

    // char printIntrpt[20];
    // snprintf(printIntrpt, 20, "Spd = %2d", windSpdRaw / 2);
    // lcd_write_string(printIntrpt);
    // _delay_ms(100);
    


    if (PINC & (1 << PC3)){ //code for rain gauge, linked to PC3

    } 

    
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


double windVane(){ 
    //char printVane[20];
    ADCSRA |= (1 << ADSC); // Starts ADC sample cycle
    while((ADCSRA & (1 << ADSC)) != 0){ // Waits for ADC sample to return
        
    }
    windDirRaw = ADCH;

    // snprintf(printVane, 20, "ADC=%8X, wDR = %03D", ADCH, windDirRaw);
    // lcd_write_string(printVane);
    // _delay_ms(1000);
    // lcd_clear_screen();
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

    // if((windDirRaw <= 200) && (windDirRaw >= 190)){
    //     lcd_write_string("Wind Dir = N");
    // }
    // else if((windDirRaw <= 120) && (windDirRaw >= 110)){
    //     lcd_write_string("Wind Dir = NE");
    // }
    // else if((windDirRaw <= 30) && (windDirRaw >= 20)){
    //     lcd_write_string("Wind Dir = E");
    // }
    // else if((windDirRaw <= 50) && (windDirRaw >= 40)){
    //     lcd_write_string("Wind Dir = SE");
    // }
    // else if((windDirRaw <= 80) && (windDirRaw >= 70)){
    //     lcd_write_string("Wind Dir = S");
    // }
    // else if((windDirRaw <= 160) && (windDirRaw >= 150)){
    //     lcd_write_string("Wind Dir = SW");
    // }
    // else if((windDirRaw <= 240) && (windDirRaw >= 230)){
    //     lcd_write_string("Wind Dir = W");
    // }
    // else if((windDirRaw <= 225) && (windDirRaw >= 215)){
    //     lcd_write_string("Wind Dir = NW");
    // }
    // else{
    //     lcd_write_string("Wind Dir = XX");
    // }
    
    // _delay_ms(1000);
    return windDirRaw;
}

double windSpeed(){
    windSpd = windSpdRaw;
    return windSpd;
}

        




