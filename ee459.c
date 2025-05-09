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

#define F_CPU 7372800UL

#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <string.h>
#include "i2c.h"
#include "lcd.h"
#include "bme280.h"
#include "weatherSensors.h"
#include "radio.h"     
#include <stddef.h>
#include <avr/interrupt.h>

void io_pin_init();
void lcd_init();

int main(void)
{
    TWSR = 0; // Set prescalar for 1
    TWBR = BDIV ; // Set bit rate register
    // Your program goes here
    //init stuff here
    io_pin_init();
    radio_init();
    lcd_init();
    sei();
    // write_reg(REG_DIO_MAPPING1, 2 << 6);  // writes 0b1000_0000 = 0x80


    // write_reg(REG_OPMODE, MODE_CONTINUOUS_WAVE);  // 0x98
    dio0_scope_pulse_loop(100);

    // Debug prints for radio registers
    lcd_clear_screen();
    lcd_write_string("Past radio init!");
    _delay_ms(1000);
    radio_debug_print_register(0x2F); // Should print 0x2D
    radio_debug_print_register(0x30); // Should print 0xD4
    radio_debug_print_register(0x01); // Should print 0x04


    //lcd_init();

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
    int windDir;
    int windSpd;

    while(1) {
        //status = bme280_get_status();
        //bme280_trigger_forced_measurement(); One of these statements is causing the loop to freeze
        // windDir = windVane();
        windSpd = windSpeed();

        // snprintf(print_data, 20, "Deg = %3d", windDir);
        snprintf(print_data, 20, "Deg = %3d", windDir);
        lcd_write_string(print_data);
        //bme280_print_reg(BME280_CTRL_MEAS_REG_ADDR);



        // if (bme280_get_status()){
        //     minutes++;
        //     snprintf(print_data, 20, "times sampled=%02d", minutes);
        //     lcd_write_string(print_data);
        //     lcd_clear_screen();
        // }
        // while (bme280_get_status()) {
        //     _delay_ms(1); // Wait until measuring bit is cleared
        //     lcd_write_string("HELOOOOOO");
        //     _delay_ms(2000);
        // }
        //lcd_clear_screen();
        
        //bme280_print_reg(BME280_CTRL_MEAS_REG_ADDR);
        // lcd_print_uint("hi", num);

        
        //lcd_write_string("End of loop");
        _delay_ms(1000);
        lcd_clear_screen();
        
        #ifdef NODE_TX
            lcd_clear_screen();
            lcd_write_string("entered radio TX!");
            radio_send("HELLO");
            lcd_clear_screen();
            lcd_write_string("radio TX!");
            _delay_ms(1000);
        #elif defined(NODE_RX)
            radio_rx_poll();  // Poll and display
        #endif

        radio_debug_print_register(0x2F); // Should print 0x2D
        radio_debug_print_register(0x30); // Should print 0xD4
        radio_debug_print_register(0x01); // Should print 0x04
        
    }
    
    return 0;   /* never reached */
}

void io_pin_init(){
    // init everything as outputs ( PB0-PB5, PB7, PC0-PC5, PD0-PD7 )
    DDRD = 0xFF;  // 0xFF = 1111 1111 (all bits set to 1)
    DDRC = (1 << PC5 | 1 << PC4 | 1 << PC0); // 0011 0001 PC3,2,1 to input
    // DDRB = 0xBF; // 1011 1111
    // don't work : PB3, PC0, PD

    PORTD &= ~(0xFF);
    PORTC &= ~(0x3F);
    PORTB &= ~(0xBF);

    // Ensure PD2 (INT0/DIO0) remains input with pull-up
    DDRD &= ~(1 << PD2);
    PORTD |=  (1 << PD2);

    // after your other PORTC setup:
    DDRC   &= ~(1<<PC0);   // make EN a high-impedance input
    PORTC  |=  (1<<PC0);   // pull it up to VIN so the regulator stays on

    DDRB |= (1<<PB3) | (1<<PB5) | (1<<PB2);  // PB3=MOSI, PB5=SCK, PB2=CS
    DDRB &= ~(1<<PB4);                       // PB4=MISO must be input


}

