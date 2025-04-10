#include "i2c.h"       
#include "lcd.h"
#include "bme280.h"
#include <util/delay.h>
#include <stdio.h>

#define DEBUG_DELAY 200
//#define INIT_DEBUG
//#define INIT_DEBUG1

void bme280_init(void);
void bme280_get_id();
uint8_t bme280_read_reg(uint8_t reg);
uint8_t bme280_get_status();
void bme280_print_reg(uint8_t reg_addr);

void bme280_init(void){
    #ifdef INIT_DEBUG
    lcd_write_string("Initializing BME280");
    _delay_ms(200);
    lcd_clear_screen();
    #endif

    // Initializing settings based on what the datasheet recommended for weather monitoring
    // Description: Only a very low data rate is needed. Power consumption is minimal. Noise of
    // pressure values is of no concern. Humidity, pressure and temperature are monitored.

    //Suggested settings for weather monitoring
        // Sensor mode forced mode ( perform one measurement, store results and return to sleep mode) 1 sample / minute
            // sensor mode: bits 1,0 in the CTRL_MEAS reg
            // mode[1:0] = 01 and 10 are FORCED MODE
        // Oversampling settings pressure ×1, temperature ×1, humidity ×1
            // humidity : bits 2,1,0 in CTRL_HUM
            // osrs_h[2:0] = 001 humidity oversampling x1
            // pressure : bits 4,3,2 in the CTRL_MEAS reg
            // osrs_p[2:0] = 001 pressure oversampling x1
            // temperature : bits 7,6,5 in the CTRL_MEAS reg
            // osrs_p[2:0] = 001 temperature oversampling x1
        // IIR filter settings filter off
            // IIR filter : bits 4,3,2 in the CONFIG register
            // filter[2:0] = 000 for filter off
    // Performance for suggested settings
        // Current consumption 0.16 µA
        // RMS Noise 3.3 Pa / 30 cm, 0.07 %RH
        // Data output rate 1/60 Hz

    #ifdef INIT_DEBUG
    lcd_write_string("here1");
    _delay_ms(500);
    lcd_clear_screen();
    #endif

    uint8_t reg;

    // HUMIDITY
    reg = BME280_CTRL_HUM_REG_ADDR;  
    uint8_t ctrl_hum;
    i2c_io(BME280_I2C_ADDR, &reg, 1, &ctrl_hum, 1);

    ctrl_hum &= ~0x07;  // Clear bits 2:0
    ctrl_hum |= 0x01;   // Set humidity oversampling x1 (001)

    uint8_t write_buf2[2] = {BME280_CTRL_HUM_REG_ADDR, ctrl_hum}; // write the buffer back
    i2c_io(BME280_I2C_ADDR, write_buf2, 2, NULL, 0);

    #ifdef INIT_DEBUG
    lcd_write_string("here2");
    _delay_ms(500);
    lcd_clear_screen();
    #endif


    // TEMP, PRES, MODE
    uint8_t ctrl_meas = 0;

    // Read current register
    reg = BME280_CTRL_MEAS_REG_ADDR;
    i2c_io(BME280_I2C_ADDR, &reg, 1, &ctrl_meas, 1);

    // Clear the fields we care about
    ctrl_meas &= ~(BME280_OSRS_T_MASK | BME280_OSRS_P_MASK | BME280_MODE_MASK);  // Clear osrs_t, osrs_p, mode

    // Set values
    ctrl_meas |= (1 << 5);  // osrs_t = x1
    ctrl_meas |= (1 << 2);  // osrs_p = x1
    ctrl_meas |= 0x01;      // mode = forced mode (01)


    // Write once
    uint8_t write_buf1[2] = {BME280_CTRL_MEAS_REG_ADDR, ctrl_meas};
    i2c_io(BME280_I2C_ADDR, write_buf1, 2, NULL, 0);


    #ifdef INIT_DEBUG1
    lcd_write_string("here3");
    _delay_ms(500);
    lcd_clear_screen();
    #endif
    // IIR FILTER
    reg = BME280_CONFIG_REG_ADDR;  
    uint8_t config_reg;
    i2c_io(BME280_I2C_ADDR, &reg, 1, &config_reg, 1);


    config_reg &= ~0x1C;  // Clear bits 4:2 00011100, turn off IIR FILTER (000)

    uint8_t write_buf3[2] = {BME280_CONFIG_REG_ADDR, config_reg}; // write the buffer back
    i2c_io(BME280_I2C_ADDR, write_buf3, 2, NULL, 0);


    lcd_write_string("bme280 initialized");
    _delay_ms(500);
    lcd_clear_screen();


}

void bme280_get_id(){
    uint8_t reg = BME280_ID_REG_ADDR;
    uint8_t id_data;
    char print_data[32];
    i2c_io(BME280_I2C_ADDR, &reg, 1, &id_data, 1);

    _delay_ms(10);

    snprintf(print_data, 32, "ID=0x%02X", id_data);

    lcd_write_string(print_data);
}

uint8_t bme280_get_status(){
    uint8_t reg = BME280_STATUS_REG_ADDR;
    uint8_t status_data;

    i2c_io(BME280_I2C_ADDR, &reg, 1, &status_data, 1);

    return status_data & (1<<3);
}

// pass in a register address, prints the contents in that register
void bme280_print_reg(uint8_t reg_addr){
    // read contents at reg_addr and store in reg_to_print
    uint8_t reg_to_print = bme280_read_reg(reg_addr);

    // print reg_to_print
    char print_data[32];
    snprintf(print_data, 32, "config=0x%02X", reg_to_print);
    lcd_write_string(print_data);

}

uint8_t bme280_read_reg(uint8_t reg){
    uint8_t reg_readout;
    i2c_io(BME280_I2C_ADDR, &reg, 1, &reg_readout, 1);
    return reg_readout;
}


