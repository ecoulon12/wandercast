/********************************************
*
*  Name:
*  Section:
*  Assignment:
*
********************************************/

#define I2C_ADDRESS 0x3D //0b0111101
#define NULL 

#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <string.h>
#include "i2c.h"

void io_pin_init();
void lcd_init();

int main(void)
{
    // Your program goes here
    //init stuff here
    io_pin_init();
    lcd_init();
    

    while(1) {
        
        
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

void lcd_init(){

    //     Commands sent to the Crystalfontz I2C LCDs are a two byte transfer that consist
    // of a control byte followed by a command byte containing the command value.
    // Control byte has bit 7 (Co) set to 1 to indicate more commands can follow, and
    // bit 6 (A0) cleared to 0 to put the data value in the command register, so this
    // should be a 0x80 value.  The command byte is the normal command value used with
    // the HD44780 controller IC.  For example, the command to clear the screen is a
    // 0x01.  To send this, the I2C write consists of two bytes: 0x80 (control byte
    // with Co=1 and A0=0) and 0x01 (clear command).

    // Data to be displayed on the LCD is sent with the control byte followed by any
    // number of data bytes containing the ASCII codes to be displayed.  Control byte
    // has bit 7 (Co) cleared to 0 to indicate more data to follow, and bit 6 (A0) set
    // to 1 to put the following bytes in the data register, so this should be a 0x40
    // value.  For example, to show "Hello", the I2C write consists of six bytes:
    // 0x40, 0x48, 0x65, 0x6C, 0x6C, 0x6f.

    // delay 500ms            Probably not needed but can't hurt
    _delay_ms(500);
	// Send the byte 0x38     Function Set: 2 lines
    // 0x80 indicates we are writing commands
    i2c_io(I2C_ADDRESS, 0x80, 8, NULL, 0);
    i2c_io(I2C_ADDRESS, 0x38, 8, NULL, 0);
	// delay 120us
    _delay_us(120);
	// Send the byte 0x0f     Display on, cursor on, cursor blinks
    i2c_io(I2C_ADDRESS, 0x80, 8, NULL, 0);
    i2c_io(I2C_ADDRESS, 0x0f, 8, NULL, 0);
	// delay 120us
    _delay_us(120);
	// Send the byte 0x01     Clear display
    i2c_io(I2C_ADDRESS, 0x80, 8, NULL, 0);
    i2c_io(I2C_ADDRESS, 0x01, 8, NULL, 0);
	// delay 15ms
    _delay_ms(15);
	// Send the byte 0x06     Entry mode: cursor shifts right
    i2c_io(I2C_ADDRESS, 0x80, 8, NULL, 0);
    i2c_io(I2C_ADDRESS, 0x06, 8, NULL, 0);
	// delay 120us
    _delay_us(120);

    //write something to the screen initially
    // 0x40 indicates we are writing data
    i2c_io(I2C_ADDRESS, 0x40, 8, NULL, 0);
    i2c_io(I2C_ADDRESS, 0x48, 8, NULL, 0);
    i2c_io(I2C_ADDRESS, 0x65, 8, NULL, 0);
    i2c_io(I2C_ADDRESS, 0x6C, 8, NULL, 0);
    i2c_io(I2C_ADDRESS, 0x6C, 8, NULL, 0);
    i2c_io(I2C_ADDRESS, 0x6F, 8, NULL, 0);

}
