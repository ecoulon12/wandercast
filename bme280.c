#include "i2c.h"       // Needed for i2c_io()
#include "lcd.h"
#include "bme280.h"
#include <util/delay.h>

#define DEBUG_DELAY 200

// from the datasheet:
// Writing is done by sending the slave address in write mode (RW = ‘0’), resulting in slave
// address 111011X0 (‘X’ is determined by state of SDO pin. Then the master sends pairs of
// register addresses and register data. The transaction is ended by a stop condition. This is
// depicted in Figure 9

void bme280_init(void);
void bme280_get_id();
void bme280_read_reg(const char *str);

void bme280_init(void){
    lcd_write_string("Initializing BME280");
    _delay_ms(200);
    lcd_clear_screen();

}

void bme280_get_id(){
    uint8_t reg = BME280_ID_REG_ADDR;
    uint8_t id_data;
    char print_data[9];
    i2c_io(BME280_I2C_ADDR, &reg, 1, &id_data, 1);

    _delay_ms(10);

    snprintf(print_data, 9, "ID=0x%02X", id_data);

    lcd_write_string(print_data);
}

void bme280_read_reg(const char *str){

}

// To be able to read registers, first the register address must be sent in write mode (slave address
//     111011X0). Then either a stop or a repeated start condition must be generated. After this the
//     slave is addressed in read mode (RW = ‘1’) at address 111011X1, after which the slave sends
//     out data from auto-incremented register addresses until a NOACKM and stop condition occurs.
//     This is depicted in Figure 10, where register 0xF6 and 0xF7 are read. 

