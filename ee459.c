/********************************************
*
*  Name: Erin Coulon, Vic Nunez, Drew Uramoto
*  Section: EE459L
*  Assignment: Wandercast
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
#include "bme280.h"
#include "weatherSensors.h"

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
    _delay_ms(1000);
    lcd_clear_screen();
    bme280_init();
    weatherSensors_init();
    lcd_clear_screen();



    uint8_t minutes = 0;
    char print_data[20];
    snprintf(print_data, 20, "times sampled=%02d", minutes);
    lcd_write_string(print_data);
    uint8_t status;
    //lcd_write_string("after sample");
    _delay_ms(500);
    lcd_clear_screen();
    // bme280_print_reg(BME280_CTRL_MEAS_REG_ADDR);
    // while(1){}
    
    //variables
    double windDir;
    double windSpd;

    while(1) {
        //status = bme280_get_status();
        //bme280_trigger_forced_measurement(); One of these statements is causing the loop to freeze
        //windDir = windVane();
        // windSpd = windSpeed();
        // snprintf(print_data, 20, "windspd = %02d", windSpd);
        // lcd_write_string(print_data);
        //bme280_print_reg(BME280_CTRL_MEAS_REG_ADDR);



        // if (bme280_get_status()){
        //     minutes++;
        //     snprintf(print_data, 20, "times sampled=%02d", minutes);
        //     lcd_write_string(print_data);
        //     lcd_clear_screen();
        // }
        while (bme280_get_status()) {
            _delay_ms(1); // Wait until measuring bit is cleared
            lcd_write_string("HELOOOOOO");
            _delay_ms(2000);
        }
        //lcd_clear_screen();
        
        //bme280_print_reg(BME280_CTRL_MEAS_REG_ADDR);
        // lcd_print_uint("hi", num);

        
        //lcd_write_string("End of loop");
        _delay_ms(1000);
        lcd_clear_screen();
        
        


        
    }
    
    return 0;   /* never reached */
}

void io_pin_init(){
    // init everything as outputs ( PB0-PB5, PB7, PC0-PC5, PD0-PD7 )
    DDRD = 0xFF;  // 0xFF = 1111 1111 (all bits set to 1)
    DDRC = (1 << PC5 | 1 << PC4 | 1 << PC0); // 0011 0001 PC3,2,1 to input
    DDRB = 0xBF; // 1011 1111
    // don't work : PB3, PC0, PD

    PORTD &= ~(0xFF);
    PORTC &= ~(0x3F);
    PORTB &= ~(0xBF);
}

