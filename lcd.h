#ifndef LCD_H
#define LCD_H

#include <stdint.h>
#include <avr/io.h>

// Define the I2C address for your LCD (8-bit format)
#define LCD_I2C_ADDRESS 0x7A

// Public functions
void lcd_init(void);
void lcd_write_string(const char *str);
void lcd_clear_screen(void);
void lcd_print_uint(const char* ,uint8_t);

#endif