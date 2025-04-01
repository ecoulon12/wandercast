/********************************************
*
*  Name:
*  Section:
*  Assignment:
*
********************************************/

#include <avr/io.h>
void io_pin_init();

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
    delay_ms(500);
	// Send the byte 0x38     Function Set: 2 lines
    i2c_io(0xA0, 0x38, 8, NULL, 0)
	// delay 120us
    delay_us(120);
	// Send the byte 0x0f     Display on, cursor on, cursor blinks
	// delay 120us
    delay_us(120);
	// Send the byte 0x01     Clear display
	// delay 15ms
    delay_ms(15);
	// Send the byte 0x06     Entry mode: cursor shifts right
	// delay 120us
    delay_us(120);
}
