/********************************************
*
*  Name:
*  Section:
*  Assignment:
*
********************************************/



#define NULL 
# define FOSC 9830400 // Clock frequency = Oscillator freq .
# define FOSC1 7372800 
# define BDIV ( FOSC1 / 200000 - 16) / 2 + 1



#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <string.h>
#include "i2c.h"
#include "lcd.h"

void io_pin_init();
void lcd_init();

int main(void)
{
    // Your program goes here
    //init stuff here
    io_pin_init();

    TWSR = 0; // Set prescalar for 1
    TWBR = BDIV ; // Set bit rate register

    lcd_init();

    lcd_clear_screen();
    lcd_write_string("Start of the program!");
    

    while(1) {
        // PORTC ^= (1<<PC0);
        // _delay_ms(500);
        
    }

    return 0;   /* never reached */
}

void io_pin_init(){
    // init everything as outputs ( PB0-PB5, PB7, PC0-PC5, PD0-PD7 )
    DDRD = 0xFF;  // 0xFF = 1111 1111 (all bits set to 1)
    DDRC = 0x3F; // 0011 1111
    DDRB = 0xBF; // 1011 1111
    // don't work : PB3, PC0, PD

    PORTD &= ~(0xFF);
    PORTC &= ~(0x3F);
    PORTB &= ~(0xBF);
}

