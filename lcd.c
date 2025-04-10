
#include "lcd.h"
#include "i2c.h"       // Needed for i2c_io()
#include <util/delay.h>
#include <string.h>

#define DEBUG_DELAY 200
// #define DEBUG_LCD


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

    // delay 500ms            Delays to help debug
    #ifdef DEBUG_LCD
    PORTC |= (1<<PC0);
    _delay_ms(DEBUG_DELAY);
    PORTC &= ~(1<<PC0);
    _delay_ms(DEBUG_DELAY);
    PORTC |= (1<<PC0);
    _delay_ms(DEBUG_DELAY);
    PORTC &= ~(1<<PC0);
    #endif

    _delay_ms(200);
	// Send the byte 0x38     Function Set: 2 lines
    // 0x80 indicates we are writing commands
    i2c_io(LCD_I2C_ADDRESS, (uint8_t[]){0x80, 0x38}, 2, NULL, 0);
    //i2c_io(I2C_ADDRESS, 0x38, 3, NULL, 0);
	// delay 120us
    _delay_us(120);
	// Send the byte 0x0f     Display on, cursor on, cursor blinks
    i2c_io(LCD_I2C_ADDRESS, (uint8_t[]){0x80, 0x0F}, 2, NULL, 0);
	// delay 120us
    _delay_us(120);
	// Send the byte 0x01     Clear display
    i2c_io(LCD_I2C_ADDRESS, (uint8_t[]){0x80, 0x01}, 2, NULL, 0);
	// delay 15ms
    _delay_ms(15);
	// Send the byte 0x06     Entry mode: cursor shifts right
    i2c_io(LCD_I2C_ADDRESS, (uint8_t[]){0x80, 0x06}, 2, NULL, 0);
	// delay 120us
    _delay_us(120);

    // write something to the screen initially
    // 0x40 indicates we are writing data

    _delay_us(120);

}

void lcd_write_string(const char* str){
    uint8_t buf[32];
    int len = strlen(str);
    buf[0] = 0x40; // control byte for data
    memcpy(&buf[1], str, len);
    i2c_io(LCD_I2C_ADDRESS, buf, len + 1, NULL, 0);
}

void lcd_clear_screen(){
    i2c_io(LCD_I2C_ADDRESS, (uint8_t[]){0x80, 0x01}, 2, NULL, 0);
    _delay_ms(15);
}

void lcd_print_uint(const char *label, uint8_t num) {
    char print_data[32];
    snprintf(print_data, sizeof(print_data), "%s=0x%02X", label, num);
    lcd_write_string(print_data);
}


