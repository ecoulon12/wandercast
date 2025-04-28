#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <util/delay.h>
#include "lcd.h"


//global vars
volatile int windSpdRaw = 0;
static int windSpd = 0;
volatile double windDirRaw = 0;
static int windDirDeg = 0 ;
volatile uint16_t timer2_count = 0; //counter to keep track of when it has been 4 seconds
volatile uint16_t rain_count = 0; // how many times the rain interrupt has been triggered
static bool windFlag = 0;
static bool rainFlag = 0;


ISR(TIMER2_COMPA_vect)
{
    // windSpd = (windSpdRaw / 10);
    // if((windSpdRaw % 5 == 0) && (windSpd != 0)){
    //     windSpd += 2;
    // }
    
    //  //runs every 4 seconds, 2.4km windspeed @ 1 tick/sec
    // char printIntrpt[20];
    // snprintf(printIntrpt, 20, "Spd = %2d", windSpdRaw / 2);
    // lcd_write_string(printIntrpt);
    // _delay_ms(100);
    timer2_count++;
    if (timer2_count >=244){
        timer2_count = 0;
        windSpd = (windSpdRaw / 10);
        if((windSpdRaw % 5 == 0) && (windSpd != 0)){
            windSpd += 2;
        }
        windSpdRaw = 0;
        //lcd_clear_screen();
        //lcd_write_string("hello wind speed");
    }

    // windSpdRaw = 0;
}


ISR(PCINT1_vect)
{
    
    if ((PINC & (1 << PC2)) && (!windFlag)) {
        // Rising edge detected
        windSpdRaw++;
        windFlag = true;
    }
    else if (!(PINC & (1 << PC2)) && (windFlag)) {
        // Falling edge detected, ready for next rising edge
        windFlag = false;
    }

    if ((PINC & (1 << PC3)) && (!rainFlag)) {
        // rain gauge handling (not implemented yet)
        // Rain gauge sends a pulse for every 0.2794mm rain detected
        // Need to scale up by 10 to make it in int. this means we will return um
        rain_count++;
    }else if (!(PINC & (1 << PC3)) && (rainFlag)){
        rainFlag = false;
    }

    
}

void weatherSensors_init(){
    sei();
    // Timer2 init
    TCCR2A |= (1 << WGM21);   // CTC mode
    TCCR2B |= (1 << CS22) | (1 << CS21) | (1 << CS20); // Prescaler 1024
    OCR2A = 255;  // (for ~100ms interrupt at 16MHz CPU) or tune as needed
    TIMSK2 |= (1 << OCIE2A);  // Enable Output Compare Match A Interrupt


    // TCCR1B |= (1 << WGM12);     // Set for CTC mode.  OCR1A = modulus
    // TIMSK1 |= (1 << OCIE1A);    // Enable CTC interrupt
    //  sei();                      // Enable global interrupts
    // OCR1A = 38400;              // Set the counter modulus (for 0.25hz)
    // TCCR1B |= ((1 << CS10) | (1 << CS12));      // Set prescaler for divide by 1024,
    //                                             // also starts timer
    PCICR |= (1 << PCIE1);  // Enable PCINT on Port C
    PCMSK1 |= (1 << PCINT10 | 1 << PCINT11); // Interrupt on PC2, PC3
    ADMUX |= ((1 << REFS0) | (1 << ADLAR) | (1 << MUX0)); /*  -Sets ADC range from 0-5V (REFS register)
                                                        -Sets readings to 8-bit definition (ADLAR)
                                                        -Sets active read channel to PC1 (MUX register)*/
    ADCSRA |= ((1 << ADPS0) | (1 << ADPS1) | (1 << ADPS2) | (1 << ADEN)); //Sets ADC clock scalar to 128 (ADPS register) and enables ADC (ADEN)
    //lcd_write_string("weatherSensor init");
    //_delay_ms(500);
}


int windVane(){ 
    
    ADCSRA |= (1 << ADSC); // Starts ADC sample cycle
    while((ADCSRA & (1 << ADSC)) != 0){ // Waits for ADC sample to return
        
    }
    windDirRaw = ADCH;
    // char printVane[20];
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


    if((windDirRaw <= 200) && (windDirRaw >= 190)){
        //lcd_write_string("Wind Dir = N");
        windDirDeg = 0;
    }
    else if((windDirRaw <= 120) && (windDirRaw >= 110)){
        //lcd_write_string("Wind Dir = NE");
        windDirDeg = 45;
    }
    else if((windDirRaw <= 30) && (windDirRaw >= 20)){
        //lcd_write_string("Wind Dir = E");
        windDirDeg = 90;
    }
    else if((windDirRaw <= 50) && (windDirRaw >= 40)){
        //lcd_write_string("Wind Dir = SE");
        windDirDeg = 135;
    }
    else if((windDirRaw <= 80) && (windDirRaw >= 70)){
        //lcd_write_string("Wind Dir = S");
        windDirDeg = 180;
    }
    else if((windDirRaw <= 160) && (windDirRaw >= 150)){
        //lcd_write_string("Wind Dir = SW");
        windDirDeg = 225;
    }
    else if((windDirRaw <= 240) && (windDirRaw >= 230)){
        //lcd_write_string("Wind Dir = W");
        windDirDeg = 270;
    }
    else if((windDirRaw <= 225) && (windDirRaw >= 215)){
        //lcd_write_string("Wind Dir = NW");
        windDirDeg = 315;
    }
    // char printVane[20];
    // snprintf(printVane, 20, "Dir = %03d", windDirDeg);
    // lcd_write_string(printVane);
    // _delay_ms(1000);
    return windDirDeg;
}

int windSpeed(){
    windSpd = windSpdRaw;
    return windSpd;
}

int rain_um(){
    return rain_count * 279;
}

        




